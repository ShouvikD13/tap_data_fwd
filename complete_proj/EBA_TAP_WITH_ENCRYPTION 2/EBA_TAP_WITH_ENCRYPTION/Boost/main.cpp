#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/thread/thread.hpp>
#include <thread>
#include <chrono>
#include <iostream>

using namespace std;

int main()
{
    boost::asio::io_service ioservice;
    boost::thread_group thr_grp;
    boost::asio::io_service::work work(ioservice);

    ioservice.post([&ioservice]()
                   {
                    ioservice.post([]()
                    {for(int i=0; i<100; i++) cout << "SubTask2\n"; });
                    for(int i=0; i<=100; i++) cout << "Task1\n"; });

    // ioservice.post([]()
    //                {for(int i=0; i<100; i++) cout << "Task2\n"; });

    thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, &ioservice));
    thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, &ioservice));
    // thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, &ioservice));
    thr_grp.join_all();
    // thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, &ioservice));

    return 0;
}