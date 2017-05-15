#include <iostream>
#include <cstdlib>
#include <assert.h>
#include <stdint.h>
#include <algorithm>
#include "kitfox-server.h"
#include "kitfox.h"

using namespace std;
using namespace libKitFox;

server_t::server_t(MPI_Comm *KitFoxComm, MPI_Comm *InterComm, kitfox_t *KitFox) :
    kitfox_server_node_t(0,KitFox->component_id_range),
    KITFOX_COMM(KitFoxComm),
    INTER_COMM(InterComm),
    kitfox(KitFox),
    active(false),
    ready_count(0),
    server_connections(0),
    peer_connections(0),
    client_connections(0)
{
    send_buffer = new uint8_t[KITFOX_MPI_BUFFER_SIZE];
    recv_buffer = new uint8_t[KITFOX_MPI_BUFFER_SIZE];
    
    MPI_Comm_rank(*KITFOX_COMM,&rank);
}

server_t::~server_t()
{
    delete [] send_buffer;
    delete [] recv_buffer;
  
    /*while(remote_server.size()) {
        delete *remote_server.begin();
        remote_server.erase(remote_server.begin());
    }
    
    while(remote_client.size()) {
        delete *remote_client.begin();
        remote_client.erase(remote_client.begin());
    }*/
    remote_server.clear();
    remote_client.clear();
}


// Check if MPINode1 and MPINode2 have disjoint ranges of component IDs.
bool server_t::is_disjoint(kitfox_server_node_t *MPINode1, kitfox_server_node_t *MPINode2)
{
    // Check if node1 and node2 have disjoint component ID ranges.
    if(MPINode2) {
        return ((MPINode1->rank != MPINode2->rank)&&
        ((MPINode1->component_id_range.first > MPINode2->component_id_range.second)||
        ((MPINode1->component_id_range.first < MPINode2->component_id_range.first)&&
        (MPINode1->component_id_range.second < MPINode2->component_id_range.first))));
    }
    
    // If MPINode2 == NULL, check if MPINode1 and this node's server have disjoint component ID ranges.
    // Check if node1 this server are disjoint.
    return ((rank != MPINode1->rank)&&
    ((MPINode1->component_id_range.first > component_id_range.second)||
    ((MPINode1->component_id_range.first < component_id_range.first)&&
    (MPINode1->component_id_range.second < component_id_range.first))));
}


// Check if ComponentID belongs to MPINode.
bool server_t::is_routable(Comp_ID ComponentID, kitfox_server_node_t *MPINode)
{
    // Check if ComponentID is routable to MPINode
    if(MPINode) {
        return (ComponentID >= MPINode->component_id_range.first)&&
        (ComponentID <= (MPINode->component_id_range.first+MPINode->component_id_range.second));
    }
    
    // If MPINode == NULL, check if ComponentID belongs to this node's server.
    return (ComponentID >= component_id_range.first)&&
    (ComponentID <= (component_id_range.first+component_id_range.second));
}


// Add RemoteNode to connected server list.
void server_t::add_kitfox_server(kitfox_server_node_t *RemoteNode)
{
    // Check connection for component tree routing.
    if(!is_disjoint(RemoteNode)) {
        kitfox->error("server","<%d,%d>: invalid connection to remote KitFox server [LP=%d] <%d,%d>",
                                   component_id_range.first,component_id_range.second,RemoteNode->rank,
                                   RemoteNode->component_id_range.first,RemoteNode->component_id_range.second);
    }
    
    // Add new remote KitFox server.
    remote_server.push_back(RemoteNode);
    
#ifdef KITFOX_MPI_DEBUG
    cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: connected to remote KitFox server [LP=" << RemoteNode->rank << "]" << endl;
#endif
}


// Remove a remote node with Rank from the connected server list.
void server_t::remove_kitfox_server(int Rank)
{
    for(vector<kitfox_server_node_t*>::iterator it = remote_server.begin(); it != remote_server.end(); it++) {
        kitfox_server_node_t* remote_node = *it;
        if(remote_node->rank == Rank) {
#ifdef KITFOX_MPI_DEBUG
            cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: disconnected to remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif
            delete *it;
            remote_server.erase(it);
            break;
        }
    }
}


// Cross connect all KitFox servers for NumRanks.
void server_t::connect_server(int ServerRank, int NumRanks)
{
    if(ServerRank == rank) {
        // This server receives connection information from other remote KitFox servers.
        while(server_connections < (NumRanks-1)) { // Wait for NumRanks connections.
            recv_buffer_position = 0;
            kitfox_server_node_t mpi_source_node;
            
            MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,MPI_ANY_SOURCE,MPI_ANY_TAG,*KITFOX_COMM,&mpi_status);
            unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
            
            // Some other requests are received first. Re-prioritize the message.
            while(recv_message_type != KITFOX_MSG_TAG_SERVER_CONNECT) { reschedule(); }
            
            unpack_message(&mpi_source_node,recv_buffer,sizeof(kitfox_server_node_t),&recv_buffer_position);
            kitfox_server_node_t *remote_node = new kitfox_server_node_t(mpi_source_node);
            add_kitfox_server(remote_node);
            server_connections++;
        }
    }
    else {
        // Send the server information to "ServerRank"
        send_buffer_position = 0;
        send_message_type = KITFOX_MSG_TAG_SERVER_CONNECT;
        pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
        pack_message((kitfox_server_node_t*)this,send_buffer,sizeof(kitfox_server_node_t),&send_buffer_position);
        MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,ServerRank,rank,*KITFOX_COMM);
        peer_connections++;
    }
    ready_count = NumRanks-1;
}


// This function terminates all connections to other KitFox servers.
// Note other KitFox servers may still have connections to this KitFox server.
void server_t::disconnect_server()
{
    while(remote_server.size()) {
        // Send MPI disconnect request.
        send_buffer_position = 0;
        send_message_type = KITFOX_MSG_TAG_SERVER_DISCONNECT;
        pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
        
        kitfox_server_node_t *remote_node = *remote_server.begin();
        
#ifdef KITFOX_MPI_DEBUG
        cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << ") to remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif

        MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,remote_node->rank,rank,*KITFOX_COMM);
        
        remove_kitfox_server(remote_node->rank);
        server_connections = 0;
    }
}


// Notify all other KitFox servers that this KitFox server is ready.
void server_t::ready()
{
    // Broadcast messages.
    for(unsigned i = 0; i < remote_server.size(); i++) {
        kitfox_server_node_t *remote_node = remote_server[i];
        
        send_buffer_position = 0;
        send_message_type = KITFOX_MSG_TAG_SERVER_READY;
        pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
        MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,remote_node->rank,rank,*KITFOX_COMM);
    }
}


// This function connects two pseudo components with SupersetID and SubsetName that have
// parent-child connection but are located in different MPI ranks.
const Comp_ID server_t::connect_remote_component(Comp_ID SupersetID, string SubsetName)
{
    Comp_ID component_id = INVALID_COMP_ID;
    
    // Broadcast messages.
    for(unsigned i = 0; i < remote_server.size(); i++) {
        kitfox_server_node_t* remote_node = remote_server[i];
        
        send_buffer_position = 0;
        send_message_type = KITFOX_MSG_TAG_CONNECT_REMOTE_COMP_REQ;
        pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
        pack_message(&SupersetID,send_buffer,sizeof(Comp_ID),&send_buffer_position); // Superset component ID
        pack_message(&SubsetName[0],send_buffer,MAX_KITFOX_COMP_NAME_LENGTH,&send_buffer_position); // Subset component name
        
#ifdef KITFOX_MPI_DEBUG
        cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << " : SupersetID=" << SupersetID << ", SubsetName=" << SubsetName << ") to remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif

        // Send a request.
        MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,remote_node->rank,rank,*KITFOX_COMM);
        
        // Receive a response.
        recv_buffer_position = 0;
        MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,MPI_ANY_SOURCE,MPI_ANY_TAG,*KITFOX_COMM,&mpi_status);
        unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
        
        // Some other requests are received first. Process them later.
        while(recv_message_type != KITFOX_MSG_TAG_CONNECT_REMOTE_COMP_RES) { reschedule(); }
        
#ifdef KITFOX_MPI_DEBUG
        cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Recv(" << KITFOX_MSG_TAG_STR[recv_message_type] << ") from remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif
        
        unpack_message(&component_id,recv_buffer,sizeof(Comp_ID),&recv_buffer_position);
        if(component_id != INVALID_COMP_ID) { break; }
    }
    
    // If component_id == INVALID_COMP_ID, connecting pseudo components failed
    // possibly because there isn't a component with SubsetName.
    return component_id;
}


// This function finds a pseudo component with ComponentName that is located in different MPI rank
// and returns its Comp_ID.
const Comp_ID server_t::get_component_id(string ComponentName)
{
    Comp_ID component_id = INVALID_COMP_ID;
  
    // Broadcast messages.
    for(unsigned i = 0; i < remote_server.size(); i++) {
        kitfox_server_node_t* remote_node = remote_server[i];
        
        send_buffer_position = 0;
        send_message_type = KITFOX_MSG_TAG_GET_COMP_ID_REQ;
        pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
        pack_message(&ComponentName[0],send_buffer,MAX_KITFOX_COMP_NAME_LENGTH,&send_buffer_position);

#ifdef KITFOX_MPI_DEBUG
        cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << " : ComponentName=" << ComponentName << ") to remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif
        
        // Send a request.
        MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,remote_node->rank,rank,*KITFOX_COMM);
        
        // Receive a response.
        recv_buffer_position = 0;
        MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,MPI_ANY_SOURCE,MPI_ANY_TAG,*KITFOX_COMM,&mpi_status);
        unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
        
        // Some other requests are received first. Process them later.
        while(recv_message_type != KITFOX_MSG_TAG_GET_COMP_ID_RES) { reschedule(); }

#ifdef KITFOX_MPI_DEBUG
        cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Recv(" << KITFOX_MSG_TAG_STR[recv_message_type] << ") from remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif
        
        unpack_message(&component_id,recv_buffer,sizeof(Comp_ID),&recv_buffer_position);
        if(component_id != INVALID_COMP_ID) { break; }
    }
  
    // If component_id == INVALID_COMP_ID, there isn't a pseudo component with ComponentName.
    return component_id;
}


// This function finds a pseudo component with ComponentID that is located in different MPI rank
// and returns its string name.
string server_t::get_component_name(Comp_ID ComponentID)
{
    string component_name;
    component_name.reserve(MAX_KITFOX_COMP_NAME_LENGTH);
    
    // Broadcast messages.
    for(unsigned i = 0; i < remote_server.size(); i++) {
        kitfox_server_node_t* remote_node = remote_server[i];
        
        send_buffer_position = 0;
        send_message_type = KITFOX_MSG_TAG_GET_COMP_NAME_REQ;
        pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
        pack_message(&ComponentID,send_buffer,sizeof(Comp_ID),&send_buffer_position);

#ifdef KITFOX_MPI_DEBUG
        cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << " : ComponentID=" << ComponentID << ") to remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif
        
        // Send a request.
        MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,remote_node->rank,rank,*KITFOX_COMM);
        
        // Receive a response.
        recv_buffer_position = 0;
        MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,MPI_ANY_SOURCE,MPI_ANY_TAG,*KITFOX_COMM,&mpi_status);
        unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
        
        // Some other requests are received first. Process them later.
        while(recv_message_type != KITFOX_MSG_TAG_GET_COMP_NAME_RES) { reschedule(); }
        
#ifdef KITFOX_MPI_DEBUG
        cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Recv(" << KITFOX_MSG_TAG_STR[recv_message_type] << ") from remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif
        
        unpack_message(&component_name[0],recv_buffer,MAX_KITFOX_COMP_NAME_LENGTH,&recv_buffer_position);
        
        if(component_name != "INVALID_COMP_NAME") { break; }
        else { component_name.clear(); }
    }
    
    // If ComponentName == "INVALID_COMP_NAME", there isn't a component with ComponentName.
    return component_name;
}


// This function broadcasts the data synchronization request for all MPI ranks.
/*const int server_t::synchronize_data(Second Time, Second Period, int DataType)
{
    // Broadcast messages.
    int queue_error = KITFOX_QUEUE_ERROR_NONE;
    for(unsigned i = 0; i < remote_server.size(); i++) {
        kitfox_server_node_t *remote_node = remote_server[i];
        
        send_buffer_position = 0;
        send_message_type = KITFOX_MSG_TAG_SYNCHRONIZE_DATA;
        pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
        pack_message(&Time,send_buffer,sizeof(Second),&send_buffer_position);
        pack_message(&Period,send_buffer,sizeof(Second),&send_buffer_position);
        pack_message(&DataType,send_buffer,sizeof(int),&send_buffer_position);

#ifdef KITFOX_MPI_DEBUG
        cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << ") to remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif
        
        // Send a request.
        MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,remote_node->rank,rank,*KITFOX_COMM);
        
cout << endl << endl << "sent" << endl << endl;
        
        // Receive a response.
        irecv(KITFOX_COMM);
        //MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,remote_node->rank,rank,*KITFOX_COMM,&mpi_status);
        recv_buffer_position = 0;
        unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
        
        while(recv_message_type != KITFOX_MSG_TAG_SYNCHRONIZE_DATA_RES) { reschedule(); }
        
        int queue_error;
        unpack_message(&queue_error,recv_buffer,sizeof(int),&recv_buffer_position);
        if(queue_error != KITFOX_QUEUE_ERROR_NONE) { return queue_error; }
    }
    return queue_error;
}*/


// This function finds a pseudo component with ComponentID and synchronizes its data of DataType.
const int server_t::synchronize_data(Comp_ID ComponentID, Second Time, Second Period, int DataType)
{
    kitfox_server_node_t *remote_node = NULL;
    for(unsigned i = 0; i < remote_server.size(); i++) {
        if(is_routable(ComponentID,remote_server[i])) {
            remote_node = remote_server[i]; break;
        }
    }
    
    if(!remote_node) { return KITFOX_QUEUE_ERROR_INVALID_PSEUDO_COMPONENT; }
    
    send_buffer_position = 0;
    send_message_type = KITFOX_MSG_TAG_SYNCHRONIZE_DATA_REQ;
    pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
    pack_message(&ComponentID,send_buffer,sizeof(Comp_ID),&send_buffer_position);
    pack_message(&Time,send_buffer,sizeof(Second),&send_buffer_position);
    pack_message(&Period,send_buffer,sizeof(Second),&send_buffer_position);
    pack_message(&DataType,send_buffer,sizeof(int),&send_buffer_position);

#ifdef KITFOX_MPI_DEBUG
    cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << " : ComponentID=" << ComponentID << ", Time=" << Time << ", Period = "; 
    if(Period == MAX_TIME) { cout << "MAX_TIME"; }
    else { cout << Period; }
    cout << ", DataType=" << KITFOX_DATA_STR[DataType] << ") to remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif
    
    // Send a request.
    MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,remote_node->rank,rank,*KITFOX_COMM);
    
    // Receive a response.
    recv_buffer_position = 0;    
    MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,MPI_ANY_SOURCE,MPI_ANY_TAG,*KITFOX_COMM,&mpi_status);
    unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
    
    while(recv_message_type != KITFOX_MSG_TAG_SYNCHRONIZE_DATA_RES) { reschedule(); }
    
#ifdef KITFOX_MPI_DEBUG
    cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Recv(" << KITFOX_MSG_TAG_STR[recv_message_type] << ") from remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif
    
    int queue_error;
    unpack_message(&queue_error,recv_buffer,sizeof(int),&recv_buffer_position);
    return queue_error;
}

// This function finds a pseudo component with ComponentID and returns its data
// that matches Time, Period, and DataType information.
const int server_t::pull_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, void *Data)
{
    kitfox_server_node_t *remote_node = NULL;
    for(unsigned i = 0; i < remote_server.size(); i++) {
        if(is_routable(ComponentID,remote_server[i])) {
            remote_node = remote_server[i]; break;
        }
    }
    
    if(!remote_node) { return KITFOX_QUEUE_ERROR_INVALID_PSEUDO_COMPONENT; }
    
    send_buffer_position = 0;
    send_message_type = KITFOX_MSG_TAG_PULL_DATA_REQ;
    pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
    pack_message(&ComponentID,send_buffer,sizeof(Comp_ID),&send_buffer_position);
    pack_message(&Time,send_buffer,sizeof(Second),&send_buffer_position);
    pack_message(&Period,send_buffer,sizeof(Second),&send_buffer_position);
    pack_message(&DataType,send_buffer,sizeof(int),&send_buffer_position);

#ifdef KITFOX_MPI_DEBUG
    cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type]
    << " : ComponentID=" << ComponentID << ", Time=" << Time << ", Period=";
    if(Period == MAX_TIME) { cout << "MAX_TIME"; }
    else { cout << Period; }
    cout << ", DataType=" << KITFOX_DATA_STR[DataType] << ") to remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif
    
    // Send a request.
    MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,remote_node->rank,rank,*KITFOX_COMM);
    
    // Receive a response.
    recv_buffer_position = 0;
    MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,MPI_ANY_SOURCE,MPI_ANY_TAG,*KITFOX_COMM,&mpi_status);
    unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
    
    while(recv_message_type != KITFOX_MSG_TAG_PULL_DATA_RES) { reschedule(); }
    
#ifdef KITFOX_MPI_DEBUG
    cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Recv(" << KITFOX_MSG_TAG_STR[recv_message_type] << ") from remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif
    
    int queue_error;
    unpack_message(Data,recv_buffer,KITFOX_DATA_SIZE[DataType],&recv_buffer_position);
    unpack_message(&queue_error,recv_buffer,sizeof(int),&recv_buffer_position);
    
    // If queue_error != KITFOX_QUEUE_ERROR_NONE, there is a problem with pulling data.
    return queue_error;
}


// This function finds a pseudo component with ComponentID and inserts a new data
// with Time, Period, and DataType information.
const int server_t::push_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, void *Data)
{
    kitfox_server_node_t *remote_node = NULL;
    for(unsigned i = 0; i < remote_server.size(); i++) {
        if(is_routable(ComponentID,remote_server[i])) {
            remote_node = remote_server[i]; break;
        }
    }
    
    if(!remote_node) { return KITFOX_QUEUE_ERROR_INVALID_PSEUDO_COMPONENT; }
    
    send_buffer_position = 0;
    send_message_type = KITFOX_MSG_TAG_PUSH_DATA_REQ;
    pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
    pack_message(&ComponentID,send_buffer,sizeof(Comp_ID),&send_buffer_position);
    pack_message(&Time,send_buffer,sizeof(Second),&send_buffer_position);
    pack_message(&Period,send_buffer,sizeof(Second),&send_buffer_position);
    pack_message(&DataType,send_buffer,sizeof(int),&send_buffer_position);
    pack_message(Data,send_buffer,KITFOX_DATA_SIZE[DataType],&send_buffer_position);

#ifdef KITFOX_MPI_DEBUG
    cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type]
    << " : ComponentID=" << ComponentID << ", Time=" << Time << ", Period=";
    if(Period == MAX_TIME) { cout << "MAX_TIME"; }
    else { cout << Period; }
    cout << ", DataType=" << KITFOX_DATA_STR[DataType] << ") to remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif
    
    // Send a request.
    MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,remote_node->rank,rank,*KITFOX_COMM);
    
    // Receive a response.
    recv_buffer_position = 0;
    MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,MPI_ANY_SOURCE,MPI_ANY_TAG,*KITFOX_COMM,&mpi_status);
    unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
    
    while(recv_message_type != KITFOX_MSG_TAG_PUSH_DATA_RES) { reschedule(); }
    
#ifdef KITFOX_MPI_DEBUG
    cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Recv(" << KITFOX_MSG_TAG_STR[recv_message_type] << ") from remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif
    
    int queue_error;
    unpack_message(&queue_error,recv_buffer,sizeof(int),&recv_buffer_position);
    
    // If queue_error != KITFOX_QUEUE_ERROR_NONE, there is a problem with pushing data.
    return queue_error;
}


// This function finds a pseudo component with ComponentID and replaces the data
// that matches with Time, Period, and DataType information.
const int server_t::overwrite_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, void *Data)
{
    kitfox_server_node_t *remote_node = NULL;
    for(unsigned i = 0; i < remote_server.size(); i++) {
        if(is_routable(ComponentID,remote_server[i])) {
            remote_node = remote_server[i]; break;
        }
    }
    
    if(!remote_node) { return KITFOX_QUEUE_ERROR_INVALID_PSEUDO_COMPONENT; }
    
    send_buffer_position = 0;
    send_message_type = KITFOX_MSG_TAG_OVERWRITE_DATA_REQ;
    pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
    pack_message(&ComponentID,send_buffer,sizeof(Comp_ID),&send_buffer_position);
    pack_message(&Time,send_buffer,sizeof(Second),&send_buffer_position);
    pack_message(&Period,send_buffer,sizeof(Second),&send_buffer_position);
    pack_message(&DataType,send_buffer,sizeof(int),&send_buffer_position);
    pack_message(Data,send_buffer,KITFOX_DATA_SIZE[DataType],&send_buffer_position);

#ifdef KITFOX_MPI_DEBUG
    cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type]
    << " : ComponentID=" << ComponentID << ", Time=" << Time << ", Period=";
    if(Period == MAX_TIME) { cout << "MAX_TIME"; }
    else { cout << Period; }
    cout << ", DataType=" << KITFOX_DATA_STR[DataType] << ") to remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif
    
    // Send a request.
    MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,remote_node->rank,rank,*KITFOX_COMM);
    
    // Receive a response.
    recv_buffer_position = 0;
    MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,MPI_ANY_SOURCE,MPI_ANY_TAG,*KITFOX_COMM,&mpi_status);
    unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
    
    while(recv_message_type != KITFOX_MSG_TAG_OVERWRITE_DATA_RES) { reschedule(); }
    
#ifdef KITFOX_MPI_DEBUG
    cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Recv(" << KITFOX_MSG_TAG_STR[recv_message_type] << ") from remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif
    
    int queue_error;
    unpack_message(&queue_error,recv_buffer,sizeof(int),&recv_buffer_position);
    
    // If queue_error != KITFOX_QUEUE_ERROR_NONE, there is a problem with overwriting data.
    return queue_error;
}


const bool server_t::update_library_variable(Comp_ID ComponentID, int VariableType, int VariableSize, void *Value, bool IsLibraryVariable, bool Wait)
{
    kitfox_server_node_t *remote_node = NULL;
    for(unsigned i = 0; i < remote_server.size(); i++) {
        if(is_routable(ComponentID,remote_server[i])) {
            remote_node = remote_server[i]; break;
        }
    }
    
    if(!remote_node) { return false; }
    
    send_buffer_position = 0;
    send_message_type = KITFOX_MSG_TAG_UPDATE_LIBRARY_VARIABLE_REQ;
    pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
    pack_message(&VariableType,send_buffer,sizeof(int),&send_buffer_position);
    pack_message(&VariableSize,send_buffer,sizeof(int),&send_buffer_position);
    pack_message(Value,send_buffer,VariableSize,&send_buffer_position);
    pack_message(&IsLibraryVariable,send_buffer,sizeof(bool),&send_buffer_position);
    pack_message(&Wait,send_buffer,sizeof(bool),&send_buffer_position);
    
#ifdef KITFOX_MPI_DEBUG
    cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type]
    << " : ComponentID=" << ComponentID << ", VariableType=";
    if(IsLibraryVariable) cout << VariableType;
    else cout << KITFOX_DATA_STR[VariableType];
    cout << ", IsLibraryVariable=" << (const char*)(IsLibraryVariable?"True":"False") << endl;
#endif

    // Send a request
    MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,remote_node->rank,rank,*KITFOX_COMM);
    
    bool updated = true;
    
    if(Wait) {
        // Receive a response.
        recv_buffer_position = 0;
        MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,MPI_ANY_SOURCE,MPI_ANY_TAG,*KITFOX_COMM,&mpi_status);
        unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
        
        while(recv_message_type != KITFOX_MSG_TAG_UPDATE_LIBRARY_VARIABLE_RES) { reschedule(); }
        
#ifdef LIBKITFOX
        cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Recv(" << KITFOX_MSG_TAG_STR[recv_message_type] << ") from remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif

        unpack_message(&updated,recv_buffer,sizeof(bool),&recv_buffer_position);
    }
    return updated;
}


// This function finds a pseudo component with ComponentID and calculates the power.
const bool server_t::calculate_power(Comp_ID ComponentID, Second Time, Second Period, counter_t Counter, bool IsTDP, bool Wait)
{
    kitfox_server_node_t *remote_node = NULL;
    for(unsigned i = 0; i < remote_server.size(); i++) {
        if(is_routable(ComponentID,remote_server[i])) {
            remote_node = remote_server[i]; break;
        }
    }
    
    if(!remote_node) { return false; }
    
    send_buffer_position = 0;
    send_message_type = KITFOX_MSG_TAG_CALCULATE_POWER_REQ;
    pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
    pack_message(&ComponentID,send_buffer,sizeof(Comp_ID),&send_buffer_position);
    pack_message(&Time,send_buffer,sizeof(Second),&send_buffer_position);
    pack_message(&Period,send_buffer,sizeof(Second),&send_buffer_position);
    pack_message(&Counter,send_buffer,sizeof(counter_t),&send_buffer_position);
    pack_message(&IsTDP,send_buffer,sizeof(bool),&send_buffer_position);
    pack_message(&Wait,send_buffer,sizeof(bool),&send_buffer_position);
    
#ifdef KITFOX_MPI_DEBUG
    cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type]
    << " : ComponentID=" << ComponentID << ", Time=" << Time << ", Period=" << Period << ", IsTDP=" << (const char*)(IsTDP?"True":"False") << ") to remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif

    // Send a request.
    MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,remote_node->rank,rank,*KITFOX_COMM);
    
    if(Wait) {
        // Receive a response.
        recv_buffer_position = 0;
        MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,MPI_ANY_SOURCE,MPI_ANY_TAG,*KITFOX_COMM,&mpi_status);
        unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
    
        while(recv_message_type != KITFOX_MSG_TAG_CALCULATE_POWER_RES) { reschedule(); }
    
#ifdef KITFOX_MPI_DEBUG
        cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Recv(" << KITFOX_MSG_TAG_STR[recv_message_type] << ") from remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif
    }
    
    return true;
}


// This function finds a pseudo component with ComponentID and calculates the temperature.
const bool server_t::calculate_temperature(Comp_ID ComponentID, Second Time, Second Period, bool DoPowerSync, bool Wait)
{
    kitfox_server_node_t *remote_node = NULL;
    for(unsigned i = 0; i < remote_server.size(); i++) {
        if(is_routable(ComponentID,remote_server[i])) {
            remote_node = remote_server[i]; break;
        }
    }
    
    if(!remote_node) { return false; }
    
    send_buffer_position = 0;
    send_message_type = KITFOX_MSG_TAG_CALCULATE_TEMPERATURE_REQ;
    pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
    pack_message(&ComponentID,send_buffer,sizeof(Comp_ID),&send_buffer_position);
    pack_message(&Time,send_buffer,sizeof(Second),&send_buffer_position);
    pack_message(&Period,send_buffer,sizeof(Second),&send_buffer_position);
    pack_message(&DoPowerSync,send_buffer,sizeof(bool),&send_buffer_position);
    
#ifdef KITFOX_MPI_DEBUG
    cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type]
    << " : ComponentID=" << ComponentID << ", Time=" << Time << ", Period=" << Period << ", DoPowerSync=" << (const char*)(DoPowerSync?"True":"False") << ") to remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif

    // Send a request.
    MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,remote_node->rank,rank,*KITFOX_COMM);
    
    if(Wait) {
        // Receive a response.
        recv_buffer_position = 0;
        MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,MPI_ANY_SOURCE,MPI_ANY_TAG,*KITFOX_COMM,&mpi_status);
        unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
    
        while(recv_message_type != KITFOX_MSG_TAG_CALCULATE_TEMPERATURE_RES) { reschedule(); }
    
#ifdef KITFOX_MPI_DEBUG
        cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Recv(" << KITFOX_MSG_TAG_STR[recv_message_type] << ") from remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif
    }
    
    return true;
}


// This function finds a pseudo component with ComponentID and calculates the failure rate.
const bool server_t::calculate_failure_rate(Comp_ID ComponentID, Second Time, Second Period, bool DoDataSync, bool Wait)
{
    kitfox_server_node_t *remote_node = NULL;
    for(unsigned i = 0; i < remote_server.size(); i++) {
        if(is_routable(ComponentID,remote_server[i])) {
            remote_node = remote_server[i]; break;
        }
    }
    
    if(!remote_node) { return false; }
    
    send_buffer_position = 0;
    send_message_type = KITFOX_MSG_TAG_CALCULATE_FAILURE_RATE_REQ;
    pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
    pack_message(&ComponentID,send_buffer,sizeof(Comp_ID),&send_buffer_position);
    pack_message(&Time,send_buffer,sizeof(Second),&send_buffer_position);
    pack_message(&Period,send_buffer,sizeof(Second),&send_buffer_position);
    pack_message(&DoDataSync,send_buffer,sizeof(bool),&send_buffer_position);
    
#ifdef KITFOX_MPI_DEBUG
    cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type]
    << " : ComponentID=" << ComponentID << ", Time=" << Time << ", Period=" << Period << ", DoDataSync=" << (const char*)(DoDataSync?"True":"False") << ") to remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif

    // Send a request.
    MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,remote_node->rank,rank,*KITFOX_COMM);
    
    if(Wait) {
        // Receive a response.
        recv_buffer_position = 0;
        MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,MPI_ANY_SOURCE,MPI_ANY_TAG,*KITFOX_COMM,&mpi_status);
        unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
    
        while(recv_message_type != KITFOX_MSG_TAG_CALCULATE_FAILURE_RATE_RES) { reschedule(); }
    
#ifdef KITFOX_MPI_DEBUG
        cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Recv(" << KITFOX_MSG_TAG_STR[recv_message_type] << ") from remote KitFox server [LP=" << remote_node->rank << "]" << endl;
#endif
    }
    
    return true;
}


// Reschedule out-of-order requests.
void server_t::reschedule()
{
    // If the received message has a higher priority, process it first.
    if(recv_message_type > KITFOX_MSG_TAG_PRIORITY_BOUND) {
#ifdef KITFOX_MPI_DEBUG
        cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: process a message (" << KITFOX_MSG_TAG_STR[recv_message_type] << ") while waiting for a response message" << endl;
#endif
        process(KITFOX_COMM,mpi_status);
        MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,MPI_ANY_SOURCE,MPI_ANY_TAG,*KITFOX_COMM,&mpi_status);
        recv_buffer_position = 0;
        unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
        return;
    }
    
    // Reschedule the request.
    kitfox_mpi_recv_message_t *recv_message = new kitfox_mpi_recv_message_t(mpi_status,recv_buffer);
    reschedule_buffer.push_back(recv_message);
    
#ifdef KITFOX_MPI_DEBUG
    int msg_tag, buf_pos = 0;
    unpack_message(&msg_tag,recv_buffer,sizeof(int),&buf_pos);
    cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: reschedule a message (" << KITFOX_MSG_TAG_STR[msg_tag] << ")" << endl;
#endif
    
    // Receive the next message.
    //MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,mpi_status.MPI_SOURCE,mpi_status.MPI_TAG,*KITFOX_COMM,&mpi_status);
    MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,MPI_ANY_SOURCE,MPI_ANY_TAG,*KITFOX_COMM,&mpi_status);
    recv_buffer_position = 0;
    unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
}


// If there is any rescheduled requests, process them.
void server_t::reprocess()
{
    while(reschedule_buffer.size() > 0) {
        kitfox_mpi_recv_message_t *recv_message = *reschedule_buffer.begin();
        mpi_status = recv_message->mpi_status;
        memcpy(recv_buffer,recv_message->buffer,KITFOX_MPI_BUFFER_SIZE);
        delete recv_message;
        reschedule_buffer.erase(reschedule_buffer.begin());
        
#ifdef KITFOX_MPI_DEBUG
        int msg_tag, buf_pos = 0;
        unpack_message(&msg_tag,recv_buffer,sizeof(int),&buf_pos);
        cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: " << "reprocess a message (" << KITFOX_MSG_TAG_STR[msg_tag] << ")" << endl;
#endif

        process(KITFOX_COMM,mpi_status);
    }
}


// Process KitFox requests.
void server_t::process(MPI_Comm *Comm, MPI_Status MPIStatus)
{
    client_response = false;

    recv_buffer_position = 0;
    unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
    
#ifdef KITFOX_MPI_DEBUG
    cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: received a message (" << KITFOX_MSG_TAG_STR[recv_message_type] << ")" << endl;
#endif
    
    switch(recv_message_type) {
        case KITFOX_MSG_TAG_SERVER_READY: {
            if(--ready_count == 0) {
                cout << "KitFox (server) [LP=" << rank << "]: ready..." << endl;
            }
            break;
        }
        case KITFOX_MSG_TAG_SERVER_DISCONNECT: {
            if((--peer_connections+client_connections) == 0) {
                disconnect_server();
                active = false;
            }
            break;
        }
        case KITFOX_MSG_TAG_CLIENT_CONNECT: {
            client_connections++;
            cout << "KitFox (server) [LP=" << rank << "]: " << client_connections << " client(s) connected" << endl;
            remote_client.insert(kitfox_client_node_t(MPIStatus.MPI_SOURCE,MPIStatus.MPI_TAG));
            assert(client_connections == remote_client.size());
            break;
        }
        case KITFOX_MSG_TAG_CLIENT_DISCONNECT: {
            remote_client.erase(kitfox_client_node_t(MPIStatus.MPI_SOURCE,MPIStatus.MPI_TAG));
            
            if(--client_connections == 0) {
                disconnect_server();
            }
            if((peer_connections+client_connections) == 0) {
                active = false;
            }
            break;
        }
        case KITFOX_MSG_TAG_CONNECT_REMOTE_COMP_REQ: {
            Comp_ID component_id;
            char comp_name[MAX_KITFOX_COMP_NAME_LENGTH];
            unpack_message(&component_id,recv_buffer,sizeof(Comp_ID),&recv_buffer_position);
            unpack_message(comp_name,recv_buffer,MAX_KITFOX_COMP_NAME_LENGTH,&recv_buffer_position);
            component_id = kitfox->connect_remote_component(component_id,comp_name);
            
            client_response = true;
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_CONNECT_REMOTE_COMP_RES;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(&component_id,send_buffer,sizeof(Comp_ID),&send_buffer_position);
            break;
        }
        case KITFOX_MSG_TAG_GET_COMP_ID_REQ: {
            Comp_ID component_id;
            char component_name[MAX_KITFOX_COMP_NAME_LENGTH];
            unpack_message(component_name,recv_buffer,MAX_KITFOX_COMP_NAME_LENGTH,&recv_buffer_position);
            component_id = kitfox->get_component_id(component_name,Comm!=KITFOX_COMM);

            client_response = true;
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_GET_COMP_ID_RES;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(&component_id,send_buffer,sizeof(Comp_ID),&send_buffer_position);
            break;
        }
        case KITFOX_MSG_TAG_GET_COMP_NAME_REQ: {
            Comp_ID component_id;
            string component_name;
            component_name.reserve(MAX_KITFOX_COMP_NAME_LENGTH);
            unpack_message(&component_id,recv_buffer,sizeof(Comp_ID),&recv_buffer_position);
            component_name = kitfox->get_component_name(component_id,Comm!=KITFOX_COMM);
            
            client_response = true;
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_GET_COMP_NAME_RES;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(&component_name[0],send_buffer,MAX_KITFOX_COMP_NAME_LENGTH,&send_buffer_position);
            break;
        }
        case KITFOX_MSG_TAG_SYNCHRONIZE_DATA: {
            Second time, period;
            int data_type, queue_error;
            unpack_message(&time,recv_buffer,sizeof(Second),&recv_buffer_position);
            unpack_message(&period,recv_buffer,sizeof(Second),&recv_buffer_position);
            unpack_message(&data_type,recv_buffer,sizeof(int),&recv_buffer_position);
            
            queue_error = kitfox->synchronize_data(time,period,data_type,false);
            
            client_response = true;
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_SYNCHRONIZE_DATA_RES;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(&queue_error,send_buffer,sizeof(int),&send_buffer_position);
            break;
        };
        case KITFOX_MSG_TAG_SYNCHRONIZE_DATA_REQ: {
            Comp_ID component_id;
            Second time, period;
            int data_type, queue_error;
            unpack_message(&component_id,recv_buffer,sizeof(Comp_ID),&recv_buffer_position);
            unpack_message(&time,recv_buffer,sizeof(Second),&recv_buffer_position);
            unpack_message(&period,recv_buffer,sizeof(Second),&recv_buffer_position);
            unpack_message(&data_type,recv_buffer,sizeof(int),&recv_buffer_position);
            
            queue_error = kitfox->synchronize_data(component_id,time,period,data_type);
            
            client_response = true;
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_SYNCHRONIZE_DATA_RES;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(&queue_error,send_buffer,sizeof(int),&send_buffer_position);
            break;
        }
        case KITFOX_MSG_TAG_SYNCHRONIZE_AND_PULL_DATA_REQ: {
            Comp_ID component_id;
            Second time, period;
            int data_type, data_size, queue_error;
            unpack_message(&component_id,recv_buffer,sizeof(Comp_ID),&recv_buffer_position);
            unpack_message(&time,recv_buffer,sizeof(Second),&recv_buffer_position);
            unpack_message(&period,recv_buffer,sizeof(Second),&recv_buffer_position);
            unpack_message(&data_type,recv_buffer,sizeof(int),&recv_buffer_position);
            data_size = KITFOX_DATA_SIZE[data_type];
            uint8_t data[data_size];
            
            queue_error = kitfox->synchronize_data(component_id,time,period,data_type,Comm!=KITFOX_COMM);
            
            switch(data_type) {
                case KITFOX_DATA_INDEX: {
                    queue_error = kitfox->pull_data<Index>(component_id,time,period,data_type,(Index*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_TIME: 
                case KITFOX_DATA_PERIOD: {
                    queue_error = kitfox->pull_data<Second>(component_id,time,period,data_type,(Second*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_COUNT: {
                    queue_error = kitfox->pull_data<Count>(component_id,time,period,data_type,(Count*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_COUNTER: {
                    queue_error = kitfox->pull_data<counter_t>(component_id,time,period,data_type,(counter_t*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_POWER: 
                case KITFOX_DATA_TDP: {
                    queue_error = kitfox->pull_data<power_t>(component_id,time,period,data_type,(power_t*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_TEMPERATURE: {
                    queue_error = kitfox->pull_data<Kelvin>(component_id,time,period,data_type,(Kelvin*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_DIMENSION: {
                    queue_error = kitfox->pull_data<dimension_t>(component_id,time,period,data_type,(dimension_t*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_LENGTH: {
                    queue_error = kitfox->pull_data<Meter>(component_id,time,period,data_type,(Meter*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_AREA: {
                    queue_error = kitfox->pull_data<MeterSquare>(component_id,time,period,data_type,(MeterSquare*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_VOLUME: {
                    queue_error = kitfox->pull_data<MeterCube>(component_id,time,period,data_type,(MeterCube*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_VOLTAGE: 
                case KITFOX_DATA_THRESHOLD_VOLTAGE: {
                    queue_error = kitfox->pull_data<Volt>(component_id,time,period,data_type,(Volt*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_CLOCK_FREQUENCY: {
                    queue_error = kitfox->pull_data<Hertz>(component_id,time,period,data_type,(Hertz*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_FAILURE_RATE: {
                    queue_error = kitfox->pull_data<Unitless>(component_id,time,period,data_type,(Unitless*)data,Comm!=KITFOX_COMM);
                    break;
                }
                default: { kitfox->error("server","undefined data type"); }
            }
            
            client_response = true;
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_SYNCHRONIZE_AND_PULL_DATA_RES;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(data,send_buffer,data_size,&send_buffer_position);
            pack_message(&queue_error,send_buffer,sizeof(int),&send_buffer_position);
            break;
        }
        case KITFOX_MSG_TAG_PULL_DATA_REQ: {
            Comp_ID component_id;
            Second time, period;
            int data_type, data_size, queue_error;
            unpack_message(&component_id,recv_buffer,sizeof(Comp_ID),&recv_buffer_position);
            unpack_message(&time,recv_buffer,sizeof(Second),&recv_buffer_position);
            unpack_message(&period,recv_buffer,sizeof(Second),&recv_buffer_position);
            unpack_message(&data_type,recv_buffer,sizeof(int),&recv_buffer_position);
            data_size = KITFOX_DATA_SIZE[data_type];
            uint8_t data[data_size];
            
            switch(data_type) {
                case KITFOX_DATA_INDEX: {
                    queue_error = kitfox->pull_data<Index>(component_id,time,period,data_type,(Index*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_TIME: 
                case KITFOX_DATA_PERIOD: {
                    queue_error = kitfox->pull_data<Second>(component_id,time,period,data_type,(Second*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_COUNT: {
                    queue_error = kitfox->pull_data<Count>(component_id,time,period,data_type,(Count*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_COUNTER: {
                    queue_error = kitfox->pull_data<counter_t>(component_id,time,period,data_type,(counter_t*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_POWER:
                case KITFOX_DATA_TDP: {
                    queue_error = kitfox->pull_data<power_t>(component_id,time,period,data_type,(power_t*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_TEMPERATURE: {
                    queue_error = kitfox->pull_data<Kelvin>(component_id,time,period,data_type,(Kelvin*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_DIMENSION: {
                    queue_error = kitfox->pull_data<dimension_t>(component_id,time,period,data_type,(dimension_t*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_LENGTH: {
                    queue_error = kitfox->pull_data<Meter>(component_id,time,period,data_type,(Meter*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_AREA: {
                    queue_error = kitfox->pull_data<MeterSquare>(component_id,time,period,data_type,(MeterSquare*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_VOLUME: {
                    queue_error = kitfox->pull_data<MeterCube>(component_id,time,period,data_type,(MeterCube*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_VOLTAGE: 
                case KITFOX_DATA_THRESHOLD_VOLTAGE: {
                    queue_error = kitfox->pull_data<Volt>(component_id,time,period,data_type,(Volt*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_CLOCK_FREQUENCY: {
                    queue_error = kitfox->pull_data<Hertz>(component_id,time,period,data_type,(Hertz*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_FAILURE_RATE: {
                    queue_error = kitfox->pull_data<Unitless>(component_id,time,period,data_type,(Unitless*)data,Comm!=KITFOX_COMM);
                    break;
                }
                default: { kitfox->error("server","undefined data type"); }
            }
            
            client_response = true;
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_PULL_DATA_RES;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(data,send_buffer,data_size,&send_buffer_position);
            pack_message(&queue_error,send_buffer,sizeof(int),&send_buffer_position);
            break;
        }
        case KITFOX_MSG_TAG_PUSH_DATA_REQ: {
            Comp_ID component_id;
            Second time, period;
            int data_type, data_size, queue_error;
            unpack_message(&component_id,recv_buffer,sizeof(Comp_ID),&recv_buffer_position);
            unpack_message(&time,recv_buffer,sizeof(Second),&recv_buffer_position);
            unpack_message(&period,recv_buffer,sizeof(Second),&recv_buffer_position);
            unpack_message(&data_type,recv_buffer,sizeof(int),&recv_buffer_position);
            data_size = KITFOX_DATA_SIZE[data_type];
            uint8_t data[data_size];
            unpack_message(data,recv_buffer,data_size,&recv_buffer_position);
            
            switch(data_type) {
                case KITFOX_DATA_INDEX: {
                    queue_error = kitfox->push_data<Index>(component_id,time,period,data_type,(Index*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_TIME: 
                case KITFOX_DATA_PERIOD: {
                    queue_error = kitfox->push_data<Second>(component_id,time,period,data_type,(Second*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_COUNT: {
                    queue_error = kitfox->push_data<Count>(component_id,time,period,data_type,(Count*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_COUNTER: {
                    queue_error = kitfox->push_data<counter_t>(component_id,time,period,data_type,(counter_t*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_POWER:
                case KITFOX_DATA_TDP: {
                    queue_error = kitfox->push_data<power_t>(component_id,time,period,data_type,(power_t*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_TEMPERATURE: {
                    queue_error = kitfox->push_data<Kelvin>(component_id,time,period,data_type,(Kelvin*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_DIMENSION: {
                    queue_error = kitfox->push_data<dimension_t>(component_id,time,period,data_type,(dimension_t*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_LENGTH: {
                    queue_error = kitfox->push_data<Meter>(component_id,time,period,data_type,(Meter*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_AREA: {
                    queue_error = kitfox->push_data<MeterSquare>(component_id,time,period,data_type,(MeterSquare*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_VOLUME: {
                    queue_error = kitfox->push_data<MeterCube>(component_id,time,period,data_type,(MeterCube*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_VOLTAGE:
                case KITFOX_DATA_THRESHOLD_VOLTAGE: {
                    queue_error = kitfox->push_data<Volt>(component_id,time,period,data_type,(Volt*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_CLOCK_FREQUENCY: {
                    queue_error = kitfox->push_data<Hertz>(component_id,time,period,data_type,(Hertz*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_FAILURE_RATE: {
                    queue_error = kitfox->push_data<Unitless>(component_id,time,period,data_type,(Unitless*)data,Comm!=KITFOX_COMM);
                    break;
                }
                default: { kitfox->error("server","undefined data type"); }
            }
            
            client_response = true;
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_PUSH_DATA_RES;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(&queue_error,send_buffer,sizeof(int),&send_buffer_position);
            break;
        }
        case KITFOX_MSG_TAG_OVERWRITE_DATA_REQ: {
            Comp_ID component_id;
            Second time, period;
            int data_type, data_size, queue_error;
            unpack_message(&component_id,recv_buffer,sizeof(Comp_ID),&recv_buffer_position);
            unpack_message(&time,recv_buffer,sizeof(Second),&recv_buffer_position);
            unpack_message(&period,recv_buffer,sizeof(Second),&recv_buffer_position);
            unpack_message(&data_type,recv_buffer,sizeof(int),&recv_buffer_position);
            data_size = KITFOX_DATA_SIZE[data_type];
            uint8_t data[data_size];
            unpack_message(data,recv_buffer,data_size,&recv_buffer_position);
            
            switch(data_type) {
                case KITFOX_DATA_INDEX: {
                    queue_error = kitfox->overwrite_data<Index>(component_id,time,period,data_type,(Index*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_TIME: 
                case KITFOX_DATA_PERIOD: {
                    queue_error = kitfox->overwrite_data<Second>(component_id,time,period,data_type,(Second*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_COUNT: {
                    queue_error = kitfox->overwrite_data<Count>(component_id,time,period,data_type,(Count*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_COUNTER: {
                    queue_error = kitfox->overwrite_data<counter_t>(component_id,time,period,data_type,(counter_t*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_POWER:
                case KITFOX_DATA_TDP: {
                    queue_error = kitfox->overwrite_data<power_t>(component_id,time,period,data_type,(power_t*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_TEMPERATURE: {
                    queue_error = kitfox->overwrite_data<Kelvin>(component_id,time,period,data_type,(Kelvin*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_DIMENSION: {
                    queue_error = kitfox->overwrite_data<dimension_t>(component_id,time,period,data_type,(dimension_t*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_LENGTH: {
                    queue_error = kitfox->overwrite_data<Meter>(component_id,time,period,data_type,(Meter*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_AREA: {
                    queue_error = kitfox->overwrite_data<MeterSquare>(component_id,time,period,data_type,(MeterSquare*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_VOLUME: {
                    queue_error = kitfox->overwrite_data<MeterCube>(component_id,time,period,data_type,(MeterCube*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_VOLTAGE: 
                case KITFOX_DATA_THRESHOLD_VOLTAGE: {
                    queue_error = kitfox->overwrite_data<Volt>(component_id,time,period,data_type,(Volt*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_CLOCK_FREQUENCY: {
                    queue_error = kitfox->overwrite_data<Hertz>(component_id,time,period,data_type,(Hertz*)data,Comm!=KITFOX_COMM);
                    break;
                }
                case KITFOX_DATA_FAILURE_RATE: {
                    queue_error = kitfox->overwrite_data<Unitless>(component_id,time,period,data_type,(Unitless*)data,Comm!=KITFOX_COMM);
                    break;
                }
                default: { kitfox->error("server","undefined data type"); }
            }
            
            client_response = true;
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_OVERWRITE_DATA_RES;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(&queue_error,send_buffer,sizeof(int),&send_buffer_position);
            break;
        }
        case KITFOX_MSG_TAG_CALCULATE_POWER_REQ: {
            Comp_ID component_id;
            Second time, period;
            counter_t counter;
            bool is_tdp, wait;
            unpack_message(&component_id,recv_buffer,sizeof(Comp_ID),&recv_buffer_position);
            unpack_message(&time,recv_buffer,sizeof(Second),&recv_buffer_position);
            unpack_message(&period,recv_buffer,sizeof(Second),&recv_buffer_position);
            unpack_message(&counter,recv_buffer,sizeof(counter_t),&recv_buffer_position);
            unpack_message(&is_tdp,recv_buffer,sizeof(bool),&recv_buffer_position);
            unpack_message(&wait,recv_buffer,sizeof(bool),&recv_buffer_position);
            
            kitfox->calculate_power(component_id,time,period,counter,is_tdp,Comm!=KITFOX_COMM);
            
            if(wait) {
                client_response = true;
                send_buffer_position = 0;
                send_message_type = KITFOX_MSG_TAG_CALCULATE_POWER_RES;
                pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            }
            break;
        }
        case KITFOX_MSG_TAG_CALCULATE_TEMPERATURE_REQ: {
            Comp_ID component_id;
            Second time, period;
            bool do_power_sync, wait;
            unpack_message(&component_id,recv_buffer,sizeof(Comp_ID),&recv_buffer_position);
            unpack_message(&time,recv_buffer,sizeof(Second),&recv_buffer_position);
            unpack_message(&period,recv_buffer,sizeof(Second),&recv_buffer_position);
            unpack_message(&do_power_sync,recv_buffer,sizeof(bool),&recv_buffer_position);
            unpack_message(&wait,recv_buffer,sizeof(bool),&recv_buffer_position);
            
            kitfox->calculate_temperature(component_id,time,period,do_power_sync,Comm!=KITFOX_COMM);
            
            if(wait) {
                client_response = true;
                send_buffer_position = 0;
                send_message_type = KITFOX_MSG_TAG_CALCULATE_TEMPERATURE_RES;
                pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            }
            break;
        }
        case KITFOX_MSG_TAG_CALCULATE_FAILURE_RATE_REQ: {
            Comp_ID component_id;
            Second time, period;
            bool do_data_sync, wait;
            unpack_message(&component_id,recv_buffer,sizeof(Comp_ID),&recv_buffer_position);
            unpack_message(&time,recv_buffer,sizeof(Second),&recv_buffer_position);
            unpack_message(&period,recv_buffer,sizeof(Second),&recv_buffer_position);
            unpack_message(&do_data_sync,recv_buffer,sizeof(bool),&recv_buffer_position);
            unpack_message(&wait,recv_buffer,sizeof(bool),&recv_buffer_position);
            
            kitfox->calculate_failure_rate(component_id,time,period,do_data_sync,Comm!=KITFOX_COMM);
            
            if(wait) {
                client_response = true;
                send_buffer_position = 0;
                send_message_type = KITFOX_MSG_TAG_CALCULATE_FAILURE_RATE_RES;
                pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            }
            break;
        }
        case KITFOX_MSG_TAG_UPDATE_LIBRARY_VARIABLE_REQ: {
            Comp_ID component_id;
            int variable_type, variable_size;
            bool is_library_variable, wait;
            unpack_message(&component_id,recv_buffer,sizeof(Comp_ID),&recv_buffer_position);
            unpack_message(&variable_type,recv_buffer,sizeof(int),&recv_buffer_position);
            unpack_message(&variable_size,recv_buffer,sizeof(int),&recv_buffer_position);
            uint8_t value[variable_size];
            unpack_message(value,recv_buffer,variable_size,&recv_buffer_position);
            unpack_message(&is_library_variable,recv_buffer,sizeof(bool),&recv_buffer_position);
            unpack_message(&wait,recv_buffer,sizeof(bool),&recv_buffer_position);
            
            bool updated = kitfox->update_library_variable(component_id,variable_type,value,is_library_variable);
            
            if(wait) {
                client_response = true;
                send_buffer_position = 0;
                send_message_type = KITFOX_MSG_TAG_UPDATE_LIBRARY_VARIABLE_RES;
                pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
                pack_message(&updated,send_buffer,sizeof(bool),&send_buffer_position);
            }
            break;
        };
        default: {
            kitfox->error("server","unknown message type");
        }
    }
    
    if(client_response) {
#ifdef KITFOX_MPI_DEBUG
        cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << ") ";
        if(Comm == KITFOX_COMM) { cout << "to remote KitFox server [LP="; }
        else { cout << "to remote client [LP=" << MPIStatus.MPI_SOURCE << ",TAG=" << MPIStatus.MPI_TAG; }
        cout << "]" << endl;
#endif

        // The client waits for the response.
        MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,MPIStatus.MPI_SOURCE,MPIStatus.MPI_TAG,*Comm);
    }
}


void server_t::listen()
{
#ifdef KITFOX_MPI_DEBUG
    cout << "KITFOX MPI DEBUG (server) [LP=" << rank << "]: server listen start" << endl;
#endif
    
    active = true;
    MPI_Comm *comm = KITFOX_COMM;

    // Notify other KitFox servers in different MPI ranks that this KitFox is ready and launching listen loop.
    ready();
    
    while(active) {
        while(true) {
            // Probe if any MPI message has been buffered.
            if(MPI_Iprobe(MPI_ANY_SOURCE,MPI_ANY_TAG,*comm,&mpi_iprobe_flag,&mpi_status) != MPI_SUCCESS) {
                kitfox->error("server","MPI_Iprobe failed with %s",(comm==KITFOX_COMM)?"KITFOX_COMM":"INTER_COMM");
            }
            
            if(mpi_iprobe_flag) {
                // Receive an MPI message.
                MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,mpi_status.MPI_SOURCE,mpi_status.MPI_TAG,*comm,&mpi_status);
             
                // Process the received message in recv_buffer.
                process(comm,mpi_status);
            }
            else { 
                break; // No more server messages are left in the buffer.
            }
        }
        
        reprocess(); // Process rescheduled messages that are received out of order.
        
        // Ping-pong between KITFOX_COMM (communicator between servers) and INTER_COMM (communicator between server and clients).
        if((comm == KITFOX_COMM)&&(ready_count == 0)) { comm = INTER_COMM; }
        else { comm = KITFOX_COMM; }
    }
    
#ifdef KITFOX_MPI_DEBUG
    cout << "KitFox (server) [LP=" << rank << "]: stop listen" << endl;
#endif
}

