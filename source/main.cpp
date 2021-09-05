// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <iostream>
#include <string>
#include <stdint.h>


#include <fstream>
#include <cstring>
#include <assert.h>

#include <arpa/inet.h>

// Include the main libnx system header, for Switch development
#include <switch.h>
#include "json.hpp"

#include "easywsclient.hpp"
// #include "easywsclient.cpp"

using nlohmann::json;
using easywsclient::WebSocket;


std::string wsserverHost = std::string("ws://127.0.0.1:9876");
std::string username;
json messages = json::object();

SwkbdTextCheckResult validate_text(char* tmp_string, size_t tmp_string_size) {
    if (strcmp(tmp_string, "bad")==0) {
        strncpy(tmp_string, "Bad string.", tmp_string_size);
        return SwkbdTextCheckResult_Bad;
    }
    return SwkbdTextCheckResult_OK;
}

std::string getKeyboardInput(std::string header){
    Result rc;
    SwkbdConfig kbd;
    char tmpoutstr[500] = {0};
    rc = swkbdCreate(&kbd, 0);
    if (R_SUCCEEDED(rc)) {
        swkbdConfigMakePresetDefault(&kbd);
        //swkbdConfigMakePresetPassword(&kbd);
        //swkbdConfigMakePresetUserName(&kbd);
        //swkbdConfigMakePresetDownloadCode(&kbd);
        swkbdConfigSetOkButtonText(&kbd, "Submit");
        //swkbdConfigSetLeftOptionalSymbolKey(&kbd, "a");
        //swkbdConfigSetRightOptionalSymbolKey(&kbd, "b");
        swkbdConfigSetHeaderText(&kbd, header.c_str());
        //swkbdConfigSetSubText(&kbd, "Sub");
        //swkbdConfigSetGuideText(&kbd, "Guide");
        swkbdConfigSetTextCheckCallback(&kbd, validate_text);//Optional, can be removed if not using TextCheck.
        swkbdShow(&kbd, tmpoutstr, sizeof(tmpoutstr));
        swkbdClose(&kbd);
        return std::string(tmpoutstr);
    }
    return std::string("");
}

void handle_message(const std::string & message)
{
    printf("%s\n", message.c_str());
    consoleUpdate(NULL);
}

// Main program entrypoint
int main(int argc, char* argv[])
{
    Result rc=0;
    consoleInit(NULL);
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);
    socketInitializeDefault();

    padUpdate(&pad);
    u64 kDown = padGetButtonsDown(&pad);
        
    /**
     * @TODO: 3 to max 10 characters
     **/

    username = getKeyboardInput("Enter your username:");
    // username = "Slluxx";
    if (username.length() <= 3){
        printf("Username must be at least 4 characters long.\n");
        consoleUpdate(NULL);
        bool btn_pressed = false;
        while (!btn_pressed){
            svcSleepThread(1000000000);
            if (kDown){
                btn_pressed = true;
            }
        }
        return 0;
    }

    printf("\n### Username set.\n\n");
    consoleUpdate(NULL);

    /**
     * @TODO: announce username to server
     * @TODO: request chat history from server
     **/


    WebSocket::pointer ws = WebSocket::from_url(wsserverHost);

    while (appletMainLoop())
    {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        if (ws){
            if(ws->getReadyState() != WebSocket::OPEN){
                ws = WebSocket::from_url(wsserverHost);
                printf("### Reconnecting...\n");
            } else {
                ws->poll();
                ws->dispatch(handle_message);
            }
        }

        if (kDown & HidNpadButton_A) {
            std::string message = getKeyboardInput("Message");

            /**
             * @TODO: maximum length message?
             * @TODO: carriage return is spammable
             **/

            if (message.length() != 0){
                json wsMessage;
                wsMessage["type"] = "message";
                wsMessage["data"]["username"] = username;
                wsMessage["data"]["message"] = message;
                if(ws)
                    ws->send(wsMessage.dump());
            }
        }

         if (kDown & HidNpadButton_Plus)
            break;

        /**
         * @TODO: dpad up/down to scroll chat history
         **/

        consoleUpdate(NULL);
    }

    socketExit();
    consoleExit(NULL);
    return 0;
}
