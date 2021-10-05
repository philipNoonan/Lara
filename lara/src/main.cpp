#include <set>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>

#include "kinect.hpp"
#include "microphone.hpp"

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
        // for (auto it : m_connections) {
        //     m_server.send(it,msg);
        // }
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
private:
    typedef std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;

    server m_server;
    con_list m_connections;

    
};

void run_kinect(broadcast_server& ServerColor, 
                broadcast_server& ServerDepth, 
                broadcast_server& ServerInfra,
                broadcast_server& ServerMic, 
                kinect &Kinect, 
                microphone &Mic) {
    while(1){ // make this a running flag enum
        Kinect.update();
        // signal locks that frames are available
        //ServerColor.send_blob(Kinect.get_color_buffer(), Kinect.get_color_buffer_size());
        ServerDepth.send_blob(Kinect.get_depth_buffer(), Kinect.get_depth_buffer_size());
        ServerInfra.send_blob(Kinect.get_infra_buffer(), Kinect.get_infra_buffer_size());
        ServerMic.send_blob((uint8_t*)Mic.get_data().data(), Mic.get_data().size());

    }
    
}



int main() {
    kinect Kinect;
    microphone mic;
    broadcast_server serverColor;
    broadcast_server serverDepth;
    broadcast_server serverInfra;
    broadcast_server serverMic;

    // Start a thread to run the processing loop
    //thread tCol(bind(&broadcast_server::get_send_col_frames,&serverColor));
    // thread tDep(bind(&broadcast_server::get_send_dep_frames,&serverDepth));
    // thread tIr(bind(&broadcast_server::get_send_ir_frames,&serverInfra));
    std::cout << "0 " << std::endl;

    thread tCol(&broadcast_server::run, &serverColor, 9005);
    std::cout << "1 " << std::endl;
    thread tDep(&broadcast_server::run, &serverDepth, 9006);
    std::cout << "2 " << std::endl;
    thread tIr(&broadcast_server::run, &serverInfra, 9007);
    std::cout << "3 " << std::endl;
    thread tAud(&broadcast_server::run, &serverMic, 9008);
    std::cout << "4 " << std::endl;

    run_kinect(serverColor, serverDepth, serverInfra, serverMic, Kinect, mic);

}