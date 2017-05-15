#ifndef __KITFOX_SERVER_H__
#define __KITFOX_SERVER_H__

#include <vector>
#include <list>
#include <set>
#include <mpi.h>
#include "kitfox-defs.h"
#include "kitfox-net.h"

namespace libKitFox
{
    class kitfox_t;
    class server_t;
    
    // KitFox server node information
    class kitfox_server_node_t
    {
    public:
        kitfox_server_node_t() : rank(-1) {}
        kitfox_server_node_t(int Rank, std::pair<Comp_ID,Comp_ID> ComponentID) : rank(Rank), component_id_range(ComponentID) {}
        ~kitfox_server_node_t() {}
        
        int rank; // KitFox communicator MPI rank
        std::pair<Comp_ID,Comp_ID> component_id_range; // Possible range of component IDs in the KitFox server
    };
    
    // KitFox client node information
    class kitfox_client_node_t
    {
    public:
        kitfox_client_node_t() : rank(-1) {}
        kitfox_client_node_t(int Rank, int Tag) : rank(Rank), tag(Tag) {}
        ~kitfox_client_node_t() {}
        
        bool operator < (const kitfox_client_node_t &c) const
        {
            if(rank == c.rank) return (tag < c.tag);
            else return rank < c.rank;
        }
        
        int rank, tag;
    };
    
    
    class kitfox_mpi_recv_message_t
    {
    public:
        kitfox_mpi_recv_message_t(MPI_Status MPIStatus, uint8_t *recv_buffer) :
            mpi_status(MPIStatus) 
        { buffer = new uint8_t[KITFOX_MPI_BUFFER_SIZE]; memcpy(buffer,recv_buffer,KITFOX_MPI_BUFFER_SIZE); }
        ~kitfox_mpi_recv_message_t() { delete [] buffer; }
        
        MPI_Status mpi_status;
        uint8_t *buffer;
    };
    
    
    class server_t : public kitfox_server_node_t
    {
    public:
        server_t(MPI_Comm *KitFoxComm, MPI_Comm *InterComm, kitfox_t *KitFox);
        ~server_t();
        
        // Cross-connect KitFox servers.
        void connect_server(int ServerRank, int NumRanks);
        // Broadcast a message that this KitFox is configured and ready.
        void ready();
        // Server listen loop
        void listen(); 
        
        /***** Remote KitFox server node information *****/
        // Store other KitFox node's information
        void add_kitfox_server(kitfox_server_node_t *RemoteNode);
        // Remove other KitFox node's information
        void remove_kitfox_server(int Rank);
        // Get KitFox node information in MPI rank = Rank.
        //kitfox_server_node_t get_kitfox_server(int Rank);
        
        /***** KitFox functions to remote KitFox servers via MPI *****/
        // Connect two pseudo components that have parent-child relation but located across different MPI ranks.
        const Comp_ID connect_remote_component(Comp_ID SupersetID, std::string SubsetName);
        // Retrieve the Comp_ID of a pseudo component that has name=ComponentName.
        const Comp_ID get_component_id(std::string ComponentName);
        // Retrieve the string name of a pseudo component that has id=ComponentID.
        std::string get_component_name(Comp_ID ComponentID);
        // Trigger the KitFox to synchronize the data of DataType.
        //const int synchronize_data(Second Time, Second Period, int DataType);
        // Synchronize the data of DataType with a pseudo component of ComponentID and all of its sub-components.
        const int synchronize_data(Comp_ID ComponentID, Second Time, Second Period, int DataType);
        // Get the data of a pseudo component that matches the criteria.
        const int pull_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, void *Data);
        // Insert a data to a pseudo component, associated with information.
        const int push_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, void *Data);
        // Replace the data of a pseudo component that matches the criteria.
        const int overwrite_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, void *Data);
        // Change the variable of the library and update the status
        const bool update_library_variable(Comp_ID ComponentID, int VariableType, int VariableSize, void *Value, bool IsLibraryVariable, bool Wait = true);
        const bool calculate_power(Comp_ID ComponentID, Second Time, Second Period, counter_t Counter, bool IsTDP, bool Wait = true);
        const bool calculate_temperature(Comp_ID ComponentID, Second Time, Second Period, bool DoPowerSync, bool Wait = true);
        const bool calculate_failure_rate(Comp_ID ComponentID, Second Time, Second Period, bool DoDataSync, bool Wait = true);
        
    private:
        // Finish all connections to other KitFox servers.
        void disconnect_server();
        // Process KitFox messages.
        void process(MPI_Comm *Comm, MPI_Status MPIStatus);
        // Reschedule out-of-order requests.
        void reschedule();
        // Re-process out-of-order requests. 
        void reprocess(); 
        
        /***** Tree routing information *****/
        // Compare if node1 and node2 have disjoint ranges of component IDs.
        bool is_disjoint(kitfox_server_node_t *MPINode1, kitfox_server_node_t *MPINode2 = NULL);
        // Check if ComponentID falls into the ID range of the node.
        bool is_routable(Comp_ID ComponentID, kitfox_server_node_t* MPINode = NULL);
        
        // Other KitFox servers that this server has connected to.
        std::vector<kitfox_server_node_t*> remote_server;
        
        // Number of connections.
        unsigned server_connections; // Number of KitFox server connections with this KitFox like a client.
        unsigned peer_connections; // Number of KitFox client connections with this KitFox like a server.
        unsigned client_connections; // Number of simulator client connection with this KitFox like a server.
        
        // Send/recv buffer control
        int send_buffer_position;
        int recv_buffer_position;
        
        // Send/recv message type
        int send_message_type;
        int recv_message_type;
        
        // Send/recv buffers
        uint8_t *send_buffer;
        uint8_t *recv_buffer;
        std::list<kitfox_mpi_recv_message_t*> reschedule_buffer; // re-ordered messaged buffer
        
        // The request requires a response.
        bool client_response;
        
        // KITFOX MPI communicator and variables
        MPI_Comm *KITFOX_COMM; // Communicator between KitFox
        MPI_Comm *INTER_COMM; // Communicator between KitFox and simulator client.
        
        int mpi_iprobe_flag;
        MPI_Request mpi_request;
        MPI_Status mpi_status;
        
        // Server listen is active.
        bool active;
        // Number of KitFox not started yet. This should be counted down to 0 to begin receiving simulator client messages.
        unsigned ready_count; 
        
        std::set<kitfox_client_node_t> remote_client;
        
        // Energy Introspector
        friend class kitfox_t;
        kitfox_t *kitfox;
    };
    
    void* client_listen(void *s);

} // namespace libKitFox
#endif
