#include <set>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>

#include "kinect.hpp"
#include "microphone.hpp"
#include "thermal.hpp"

#include <chrono>
#include <future>  
#include <string>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using websocketpp::lib::thread;

class broadcast_server {
public:
    broadcast_server() {
        m_server.init_asio();

        m_server.set_open_handler(bind(&broadcast_server::on_open,this,::_1));
        m_server.set_close_handler(bind(&broadcast_server::on_close,this,::_1));
        m_server.set_message_handler(bind(&broadcast_server::on_message,this,::_1,::_2));
        m_server.clear_access_channels(websocketpp::log::alevel::all); 

    }

    void on_open(connection_hdl hdl) {
        m_connections.insert(hdl);
    }

    void on_close(connection_hdl hdl) {
        m_connections.erase(hdl);
    }

    void on_message(connection_hdl hdl, server::message_ptr msg) {
        // std::cout << "on_message called with hdl: " << hdl.lock().get()
        //         << " and message: " << msg->get_payload()
        //         << std::endl;

        std::string s = msg->get_payload();
        //std::cout << s << std::endl;
        std::string delimiter = " ";

        std::vector<std::string> stringCommands;

        size_t pos = 0;
        std::string token;
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            stringCommands.push_back(token);
            s.erase(0, pos + delimiter.length());
        }
        stringCommands.push_back(s);

        //for (auto i : stringCommands) {
        //    std::cout << i << std::endl;
        //}

       
            if (stringCommands[0] == "sliderIR") {
                gainIR = std::stof(stringCommands[1]);
                biasIR = std::stof(stringCommands[2]);
            }
            else if (stringCommands[0] == "color") {
                showColor = !showColor;
                //std::cout << "color " << showColor << std::endl;
            }
            else if (stringCommands[0] == "depth") {
                showDepth = !showDepth;
                //std::cout << "depth " << showDepth << std::endl;
            }
            else if (stringCommands[0] == "infra") {
                showInfra = !showInfra;
                //std::cout << "infra " << showInfra << std::endl;
            }
            else if (stringCommands[0] == "audio") {
                showAudio = !showAudio;
                //std::cout << "infra " << showInfra << std::endl;
            }
    }

    // void get_send_col_frames() {

    //     while(1) {
    //         // wait for unlock signal that color frame is ready
    //         Kinect.update();
    //         auto buf = Kinect.get_color_buffer();
    //         auto size = Kinect.get_color_buffer_size();


    //     }
    // }
    void send_blob(uint8_t* blob, size_t blobSize) {

        for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
            m_server.send(*it, blob, blobSize, websocketpp::frame::opcode::BINARY);
        }
    }

    // void get_send_dep_frames() {

    //     while(1) {
    //         Kinect.update();
    //         auto buf = Kinect.get_depth_buffer();
    //         auto size = Kinect.get_depth_buffer_size();

    //         for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
    //             m_server.send(*it, buf, size, websocketpp::frame::opcode::BINARY);
    //         }
    //     }
    // }

    // void get_send_ir_frames() {

    //     while(1) {
    //         Kinect.update();
    //         auto buf = Kinect.get_infra_buffer();
    //         auto size = Kinect.get_infra_buffer_size();

    //         for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
    //             m_server.send(*it, buf, size, websocketpp::frame::opcode::BINARY);
    //         }
    //     }
    // }

    void run(uint16_t port) {
        m_server.listen(port);
        m_server.start_accept();
        m_server.run();
    }

    float gainIR = 1;
    float biasIR = 65535;

    bool showColor = false;
    bool showDepth = false;
    bool showInfra = true;
    bool showAudio = false;

    
private:
    typedef std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;

    server m_server;
    con_list m_connections;



    
};

class devices {
private:


public:

    devices() {};
    ~devices() {};
    kinect Kinect;
    microphone Mic;
    thermal Thermal;
        
void run_kinect(broadcast_server& ServerColor, 
                broadcast_server& ServerDepth, 
                broadcast_server& ServerInfra,
                broadcast_server& ServerControls,
                kinect &Kinect
                ) {
    while(1){ // make this a running flag enum
        Kinect.set_ir_invgain_bias(ServerControls.gainIR, ServerControls.biasIR);
        Kinect.update();
        // signal locks that frames are available
        if (ServerControls.showColor) {
            ServerColor.send_blob(Kinect.get_color_buffer(), Kinect.get_color_buffer_size());
        }
        if (ServerControls.showDepth) {
            ServerDepth.send_blob(Kinect.get_depth_buffer(), Kinect.get_depth_buffer_size());
        }
        if (ServerControls.showInfra) {
            ServerInfra.send_blob(Kinect.get_infra_buffer(), Kinect.get_infra_buffer_size());
        }
    }   
}

void run_mic(broadcast_server& ServerMic, 
             broadcast_server& ServerControls) {
    while (1) {
        std::vector<float> audio_data = Mic.run_blocking();
        // https://stackoverflow.com/questions/13669094/how-to-use-stdasync-on-a-member-function
        // using async hopefully allows the ws to transmit and the thread to instantly return to the read_stream
        if (ServerControls.showAudio) {
            ServerMic.send_blob((uint8_t*)audio_data.data(), 1024 * sizeof(float));
        }
        // for propper glitchless we may need to actually use the callback and count how many chunks 
        // and chunk number to pass to the audiobuffer for propper cueing
    }
}

void run_thermal(broadcast_server& ServerThermal, 
                 broadcast_server& ServerControl) {
    while(1) {
        std::string thermal_data = Thermal.run_blocking();
        //std::cout << thermal_data << std::endl;
        ServerThermal.send_blob((uint8_t*)thermal_data.c_str(), 768 * sizeof(char));
    } 
}

};



int main() {

    devices devs;

    broadcast_server serverColor;
    broadcast_server serverDepth;
    broadcast_server serverInfra;
    broadcast_server serverMic;
    broadcast_server serverThermal;
    broadcast_server serverControls;


    thread tCol(&broadcast_server::run, &serverColor, 9005);
    thread tDep(&broadcast_server::run, &serverDepth, 9006);
    thread tIr(&broadcast_server::run, &serverInfra, 9007);
    thread tAud(&broadcast_server::run, &serverMic, 9008);
    thread tTher(&broadcast_server::run, &serverThermal, 9009);
    thread tCont(&broadcast_server::run, &serverControls, 9010);

    thread tAudServer(&devices::run_mic, &devs, std::ref(serverMic), std::ref(serverControls));
    thread tThermalServer(&devices::run_thermal, &devs, std::ref(serverThermal), std::ref(serverControls));

    devs.run_kinect(serverColor, serverDepth, serverInfra, serverControls, devs.Kinect); // ALSO THREAD ME??

}