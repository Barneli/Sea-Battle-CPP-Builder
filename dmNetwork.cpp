#include <vcl.h>
#pragma hdrstop

#include "dmNetwork.h"
#include "Lobby.h"
#include "Arrangment_ship.h"
#include "Play_with_friend.h"
#include <vector>

#include <IdGlobal.hpp>
#include <IdGlobalProtocols.hpp>
#include <IdStackConsts.hpp>
#include <IdUDPClient.hpp>
#include <IdUDPServer.hpp>
#include <IdExceptionCore.hpp>

#pragma package(smart_init)
#pragma classgroup "Vcl.Controls.TControl"
#pragma resource "*.dfm"

TDataModule1 *DataModule1;

static TIdBytes AnsiToIdBytes(const AnsiString& s)
{
    TIdBytes result;
    result.Length = s.Length();
    for (int i = 0; i < s.Length(); i++)
        result[i] = (Byte)s[i + 1];
    return result;
}

static AnsiString IdBytesToAnsi(const TIdBytes& b)
{
    AnsiString result;
    result.SetLength(b.Length);
    for (int i = 0; i < b.Length; i++)
        result[i + 1] = (char)b[i];
    return result;
}

//---------------------------------------------------------------------------
__fastcall TDataModule1::TDataModule1(TComponent* Owner)
    : TDataModule(Owner), IsHost(false), OpponentReady(false)
{
    TCPServer->OnExecute = TCPServerExecute;
}

//---------------------------------------------------------------------------
void TDataModule1::SendToOpponent(const String& msg)
{
    try {
        String line = msg + "\n";
        if (IsHost) {
            // FIX: правильний тип — TIdThreadSafeObjectList через LockList
            TIdContextThreadList *contexts = TCPServer->Contexts;
            TList *list = contexts->LockList();
            try {
                for (int i = 0; i < list->Count; i++) {
                    TIdContext *ctx = (TIdContext*)list->Items[i];
                    ctx->Connection->IOHandler->Write(line);
                }
            } __finally {
                contexts->UnlockList();
            }
        } else {
            TCPClient->IOHandler->Write(line);
        }
    } catch (...) {}
}

//---------------------------------------------------------------------------
void __fastcall TDataModule1::UDPServerUDPRead(
    TIdUDPListenerThread *AThread,
    const TIdBytes AData,
    TIdSocketHandle *ABinding)
{
    AnsiString RequestAnsi = IdBytesToAnsi(AData);
    String Request = String(RequestAnsi).Trim();

    String prefix = "FIND_GAME:";
    if (Request.SubString(1, prefix.Length()) != prefix) return;

    String CodeInRequest = Request.SubString(
        prefix.Length() + 1,
        Request.Length() - prefix.Length()).Trim();

    if (CodeInRequest != GameCode.Trim()) return;

    TIdBytes ResponseData = AnsiToIdBytes("I_AM_SERVER");
    ABinding->SendTo(ABinding->PeerIP, ABinding->PeerPort, ResponseData, Id_IPv4);
}

//---------------------------------------------------------------------------
// Допоміжний клас для Synchronize замість лямбди
class TSyncHandler : public TThread
{
public:
    String Line;
    TSyncHandler() : TThread(true) { FreeOnTerminate = false; }
    void __fastcall Execute() {}

    void __fastcall DoHandle() {
        DataModule1->HandleMessage(Line);
    }
    void __fastcall DoDisconnect() {
        if (Form6 && Form6->Visible)
            Form6->OnOpponentDisconnected();
    }
};

//---------------------------------------------------------------------------
void __fastcall TDataModule1::TCPServerExecute(TIdContext *AContext)
{
    // Перше підключення клієнта — хост переходить до розстановки
    TThread::Queue(NULL, [](){
        if (Form5 && Form5->Visible) {
            Form5->Hide();
            if (Form2) Form2->Show();
        }
    });

    // Налаштовуємо IOHandler щоб не кидав виняток при таймауті ReadLn
    AContext->Connection->IOHandler->ReadTimeout = 100; // 100мс — неблокуючий polling

    try {
        while (AContext->Connection->Connected()) {
            String line = "";
            try {
                // CheckForDisconnect перевіряє чи клієнт ще підключений
                AContext->Connection->IOHandler->CheckForDisconnect(false);
                if (AContext->Connection->IOHandler->InputBufferIsEmpty()) {
                    Sleep(50);
                    continue;
                }
                line = AContext->Connection->IOHandler->ReadLn().Trim();
            } catch (EIdReadTimeout &) {
                continue; // таймаут — просто чекаємо далі
            } catch (...) {
                break; // справжня помилка — виходимо
            }

            if (line.IsEmpty()) continue;

            String *lineCopy = new String(line);
            TThread::Queue(NULL, [lineCopy](){
                DataModule1->HandleMessage(*lineCopy);
                delete lineCopy;
            });
        }
    } catch (...) {}

    // Клієнт відключився
    TThread::Queue(NULL, [](){
        if (Form6 && Form6->Visible)
            Form6->OnOpponentDisconnected();
    });
}

//---------------------------------------------------------------------------
void TDataModule1::HandleMessage(const String& line)
{
    if (line == "READY") {
        OpponentReady = true;
        if (Form2 && Form2->Visible)
            Form2->OnOpponentReady();
        return;
    }

    if (line.SubString(1, 5) == "SHOT:") {
        String rest = line.SubString(6, line.Length() - 5);
        int colon   = rest.Pos(":");
        if (colon > 0) {
            int x = StrToIntDef(rest.SubString(1, colon - 1), -1);
            int y = StrToIntDef(rest.SubString(colon + 1, rest.Length() - colon), -1);
            if (x >= 0 && y >= 0 && Form6 && Form6->Visible)
                Form6->OnOpponentShot(x, y);
        }
        return;
    }

    if (line.SubString(1, 7) == "RESULT:") {
        // Формат: "RESULT:x:y:result:sx:sy:val:sx:sy:val..."
        // (зону навколо не передаємо — обраховуємо тут з координат корабля)
        TStringList *parts = new TStringList();
        parts->Delimiter = ':';
        parts->StrictDelimiter = true;
        parts->DelimitedText = line.SubString(8, line.Length() - 7);

        if (parts->Count < 3) { delete parts; return; }
        int x      = StrToIntDef(parts->Strings[0], -1);
        int y      = StrToIntDef(parts->Strings[1], -1);
        int result = StrToIntDef(parts->Strings[2], -1);
        if (x < 0 || y < 0 || result < 0) { delete parts; return; }

        // Клітини потопленого корабля: тріплети x:y:value з індексу 3
        std::vector<ShipCell> shipCells;
        for (int i = 3; i + 2 <= parts->Count - 1; i += 3) {
            int sx = StrToIntDef(parts->Strings[i],   -1);
            int sy = StrToIntDef(parts->Strings[i+1], -1);
            int sv = StrToIntDef(parts->Strings[i+2], -1);
            if (sx >= 0 && sy >= 0 && sv > 0) {
                ShipCell sc; sc.x = sx; sc.y = sy; sc.value = sv;
                shipCells.push_back(sc);
            }
        }
        delete parts;

        // Обраховуємо зону навколо тут — надійніше ніж передавати по мережі
        std::vector<std::pair<int,int>> aroundCells;
        if (result == 2 && !shipCells.empty()) {
            bool used[10][10] = {};
            // Помічаємо клітини корабля
            for (int k = 0; k < (int)shipCells.size(); k++)
                used[shipCells[k].x][shipCells[k].y] = true;
            // Обходимо всі 8 сусідів кожної клітини корабля
            for (int k = 0; k < (int)shipCells.size(); k++) {
                for (int ddx = -1; ddx <= 1; ddx++) {
                    for (int ddy = -1; ddy <= 1; ddy++) {
                        if (ddx == 0 && ddy == 0) continue;
                        int nx = shipCells[k].x + ddx;
                        int ny = shipCells[k].y + ddy;
                        if (nx < 0 || nx >= 10 || ny < 0 || ny >= 10) continue;
                        if (used[nx][ny]) continue;
                        used[nx][ny] = true;
                        aroundCells.push_back(std::make_pair(nx, ny));
                    }
                }
            }
        }

        if (Form6 && Form6->Visible)
            Form6->ApplyShotResult(x, y, result, shipCells, aroundCells);
        return;
    }
}

//---------------------------------------------------------------------------
void __fastcall TDataModule1::TCPClientConnected(TObject *Sender)
{
    TThread::Queue(NULL, [](){
        if (Form5 && Form5->Visible) {
            //Form5->lblStatus->Caption = "З'єднано! Переходимо до розстановки...";
            Sleep(300);
            Form5->Hide();
            if (Form2) Form2->Show();
        }
    });

    // Клієнт слухає повідомлення від хоста в окремому потоці
    TThread::CreateAnonymousThread([&](){
        Sleep(500); // чекаємо поки IOHandler повністю ініціалізується

        TCPClient->IOHandler->ReadTimeout = 100;

        try {
            while (TCPClient->Connected()) {
                String line = "";
                try {
                    TCPClient->IOHandler->CheckForDisconnect(false);
                    if (TCPClient->IOHandler->InputBufferIsEmpty()) {
                        Sleep(50);
                        continue;
                    }
                    line = TCPClient->IOHandler->ReadLn().Trim();
                } catch (EIdReadTimeout &) {
                    continue;
                } catch (...) {
                    break;
                }

                if (line.IsEmpty()) continue;

                String *lineCopy = new String(line);
                TThread::Queue(NULL, [lineCopy](){
                    DataModule1->HandleMessage(*lineCopy);
                    delete lineCopy;
                });
            }
        } catch (...) {}

        TThread::Queue(NULL, [](){
            if (Form6 && Form6->Visible)
                Form6->OnOpponentDisconnected();
        });
    })->Start();
}
