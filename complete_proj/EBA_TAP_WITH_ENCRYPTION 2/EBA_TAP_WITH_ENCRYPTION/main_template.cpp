// Include nessesary libraries here

int main(int argc, char **argv)
{

    // Declare variables here

    // Declare thread_groups here

    // Define io_contexts here

    // Define io_context::work here

    try
    {
        // Read configuration file here

        // exg_comm exg_comm_obj(exgobj_io_service, exg_endpoint, gateway_comm_obj, si_max_ords_in_window, encrypt_ctx, decrypt_ctx);

        // Create thread here, and bind the run method of the exchange io_context
        // exg_thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, &exgobj_io_service));
        // exg_thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, &exgobj_io_service));

        pipe_handler pipe_handler_obj(pipes_io_service, exg_comm_obj, pipehandlr_exgresp_io_service);

        // Create thread here, and bind the run method of the app_service io_context
        // pipe_thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, &pipes_io_service));
        // pipe_thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, &pipes_io_service));
        // pipe_thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, &pipes_io_service));

        // Create thread here, and bind the run method of the exchange_response io_context
        // pipe_resp_hdlr_thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, &pipehandlr_exgresp_io_service));
    }
    catch (std::exception &e)
    {
        // Handle errors here
    }
    catch (...)
    {
        // Handle errors here
    }

    // Declare io_context stop here

    // Use io_context join all here

    return 0;
}