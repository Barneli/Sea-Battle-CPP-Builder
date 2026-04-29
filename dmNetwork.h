//---------------------------------------------------------------------------
#ifndef dmNetworkH
#define dmNetworkH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdCustomTCPServer.hpp>
#include <IdTCPClient.hpp>
#include <IdTCPConnection.hpp>
#include <IdTCPServer.hpp>
#include <IdContext.hpp>
#include <IdUDPBase.hpp>
#include <IdUDPClient.hpp>
#include <IdUDPServer.hpp>
#include <IdGlobal.hpp>
#include <IdSocketHandle.hpp>
//---------------------------------------------------------------------------
class TDataModule1 : public TDataModule
{
__published:
    TIdUDPServer *UDPServer;
    TIdUDPClient *UDPClient;
    TIdTCPServer *TCPServer;
    TIdTCPClient *TCPClient;

    void __fastcall UDPServerUDPRead(TIdUDPListenerThread *AThread,
        const TIdBytes AData, TIdSocketHandle *ABinding);

    void __fastcall TCPServerExecute(TIdContext *AContext);
    void __fastcall TCPClientConnected(TObject *Sender);

private:
public:
    String GameCode;
    String ServerIP;
    bool   IsHost;
    bool   OpponentReady;  // суперник вже готовий?

    // Надіслати рядок суперникові
    void SendToOpponent(const String& msg);
    void HandleMessage(const String& line);

    __fastcall TDataModule1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TDataModule1 *DataModule1;
//---------------------------------------------------------------------------
#endif
