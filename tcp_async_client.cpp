using boost::asio::ip::tcp;

class TCPClient
{
    public:
        TCPClient(boost::asio::io_service& IO_Service, tcp::resolver::iterator EndPointIter);
        void Close();

    private:
        boost::asio::io_service& m_IOService;
        tcp::socket m_Socket;

        string m_SendBuffer;
        static const size_t m_BufLen = 100;
        char m_RecieveBuffer[m_BufLen*2];

        void OnConnect(const boost::system::error_code& ErrorCode, tcp::resolver::iterator EndPointIter);
        void OnReceive(const boost::system::error_code& ErrorCode);
        void OnSend(const boost::system::error_code& ErrorCode);
        void DoClose();
};

TCPClient::TCPClient(boost::asio::io_service& IO_Service, tcp::resolver::iterator EndPointIter)
: m_IOService(IO_Service), m_Socket(IO_Service), m_SendBuffer("")
{
    tcp::endpoint EndPoint = *EndPointIter;

    m_Socket.async_connect(EndPoint,
        boost::bind(&TCPClient::OnConnect, this, boost::asio::placeholders::error, ++EndPointIter));
}

void TCPClient::Close()
{
    m_IOService.post(
        boost::bind(&TCPClient::DoClose, this));
}
void TCPClient::OnConnect(const boost::system::error_code& ErrorCode, tcp::resolver::iterator EndPointIter)
{
    cout << "OnConnect..." << endl;
    if (ErrorCode == 0)
    {
        cin >> m_SendBuffer;
        cout << "Entered: " << m_SendBuffer << endl;
        m_SendBuffer += "\0";

        m_Socket.async_send(boost::asio::buffer(m_SendBuffer.c_str(),m_SendBuffer.length()+1),
            boost::bind(&TCPClient::OnSend, this,
            boost::asio::placeholders::error));
    } 
    else if (EndPointIter != tcp::resolver::iterator())
    {
        m_Socket.close();
        tcp::endpoint EndPoint = *EndPointIter;

        m_Socket.async_connect(EndPoint, 
            boost::bind(&TCPClient::OnConnect, this, boost::asio::placeholders::error, ++EndPointIter));
    }
}

void TCPClient::OnReceive(const boost::system::error_code& ErrorCode)
{
    cout << "receiving..." << endl;
    if (ErrorCode == 0)
    {
        cout << m_RecieveBuffer << endl;

        m_Socket.async_receive(boost::asio::buffer(m_RecieveBuffer, m_BufLen),
            boost::bind(&TCPClient::OnReceive, this, boost::asio::placeholders::error));
    } 
    else 
    {
        cout << "ERROR! OnReceive..." << endl;
        DoClose();
    }
}

void TCPClient::OnSend(const boost::system::error_code& ErrorCode)
{
    cout << "sending..." << endl;
    if (!ErrorCode)
    {
        cout << "\""<< m_SendBuffer <<"\" has been sent" << endl;
        m_SendBuffer = "";

        m_Socket.async_receive(boost::asio::buffer(m_RecieveBuffer, m_BufLen),
            boost::bind(&TCPClient::OnReceive, this, boost::asio::placeholders::error));
    }
    else
    {
        cout << "OnSend closing" << endl;
        DoClose();
    }

}

void TCPClient::DoClose()
{
    m_Socket.close();
}

int main()
{
    try 
    {
        cout << "Client is starting..." << endl;
        boost::asio::io_service IO_Service;

        tcp::resolver Resolver(IO_Service);

        string port = "13";
        tcp::resolver::query Query("127.0.0.1", port);

        tcp::resolver::iterator EndPointIterator = Resolver.resolve(Query);

        TCPClient Client(IO_Service, EndPointIterator);

        cout << "Client is started!" << endl;

        cout << "Enter a query string " << endl;

        boost::thread ClientThread(boost::bind(&boost::asio::io_service::run, &IO_Service));

	ClientThread.join();
        Client.Close();
    } 
    catch (exception& e)
    {
        cerr << e.what() << endl;
    }

    cout << "\nClosing";
    getch();
}

/*
 * https://stackoverflow.com/questions/12990840/boost-async-tcp-client
 * /
