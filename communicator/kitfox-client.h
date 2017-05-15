#ifndef __KITFOX_CLIENT_H__
#define __KITFOX_CLIENT_H__

#include "kitfox-defs.h"
#include "kitfox-net.h"

namespace libKitFox
{
    class client_t
    {
    public:
        client_t(MPI_Comm *InterComm, int ClientID) :
            INTER_COMM(InterComm),
            rank(-1),
            id(ClientID)
        {
            send_buffer = new uint8_t[KITFOX_MPI_BUFFER_SIZE];
            recv_buffer = new uint8_t[KITFOX_MPI_BUFFER_SIZE];
        }
        virtual ~client_t()
        {
            delete [] send_buffer;
            delete [] recv_buffer;
        }
        
        // Send a connect request to the KitFox server of rank=ServerRank via INTER_COMM.
        void connect_server(int ServerRank)
        {
            rank = ServerRank;
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_CLIENT_CONNECT;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);

#ifdef KITFOX_MPI_DEBUG
            std::cout << "KITFOX MPI DEBUG (client) [ID=" << id << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << ") to KitFox server [LP=" << rank << "]" << std::endl;    
#endif            
            
            MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,rank,id,*INTER_COMM);
        }
        
        // Send a disconnect request to the KitFox server.
        void disconnect_server()
        {
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_CLIENT_DISCONNECT;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,rank,id,*INTER_COMM);
        }
        
        // Get the ID of the pseudo component of name=ComponentName.
        Comp_ID get_component_id(std::string ComponentName)
        {
            Comp_ID component_id = INVALID_COMP_ID;
            
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_GET_COMP_ID_REQ;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(&ComponentName[0],send_buffer,MAX_KITFOX_COMP_NAME_LENGTH,&send_buffer_position);

#ifdef KITFOX_MPI_DEBUG
            std::cout << "KITFOX MPI DEBUG (client) [ID=" << id << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << " : ComponentName=" << ComponentName << ") to KitFox server [LP=" << rank << "]" << std::endl;    
#endif            
            
            // Send a request.
            MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,rank,id,*INTER_COMM);
            
            // Receive a response.
            // This is blocking until the KitFox response returns.
            recv_buffer_position = 0;
            MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,rank,id,*INTER_COMM,&mpi_status);
            unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
            
            if(recv_message_type != KITFOX_MSG_TAG_GET_COMP_ID_RES) {
                std::cout << "KITFOX ERROR (client) [ID=" << id << "]: received an unexpected message (" << KITFOX_MSG_TAG_STR[recv_message_type] << ")" << std::endl; 
                exit(EXIT_FAILURE);
            }
            
            unpack_message(&component_id,recv_buffer,sizeof(Comp_ID),&recv_buffer_position);
            return component_id;
        }
        
        // Get the string name of the pseudo component with id=ComponentID
        std::string get_component_name(Comp_ID ComponentID)
        {
            std::string component_name;
            component_name.reserve(MAX_KITFOX_COMP_NAME_LENGTH);
            
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_GET_COMP_NAME_REQ;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(&ComponentID,send_buffer,sizeof(Comp_ID),&send_buffer_position);

#ifdef KITFOX_MPI_DEBUG
            std::cout << "KITFOX MPI DEBUG (client) [ID=" << id << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << " : ComponentID=" << ComponentID << ") to KitFox server [LP=" << rank << "]" << std::endl;
#endif
            
            // Send a request.
            MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,rank,id,*INTER_COMM);
            
            // Receive a response.
            // This is blocking until the KitFox response returns.
            recv_buffer_position = 0;
            MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,rank,id,*INTER_COMM,&mpi_status);
            unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
            
            if(recv_message_type != KITFOX_MSG_TAG_GET_COMP_NAME_RES) {
                std::cout << "KITFOX ERROR (client) [ID=" << id << "]: received an unexpected message (" << KITFOX_MSG_TAG_STR[recv_message_type] << ")" << std::endl;
                exit(EXIT_FAILURE);
            }
            
            unpack_message(&component_name[0],recv_buffer,MAX_KITFOX_COMP_NAME_LENGTH,&recv_buffer_position);
            return component_name;
        }
        
        /*const int synchronize_data(Second Time, Second Period, int DataType)
        {
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_SYNCHRONIZE_DATA;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(&Time,send_buffer,sizeof(Second),&send_buffer_position);
            pack_message(&Period,send_buffer,sizeof(Second),&send_buffer_position);
            pack_message(&DataType,send_buffer,sizeof(int),&send_buffer_position);

#ifdef KITFOX_MPI_DEBUG
            std::cout << "KITFOX MPI DEBUG (client) [ID=" << id << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << " : Time=" << Time << ", Period="; 
            if(Period==MAX_TIME) { std::cout << "MAX_TIME"; }
            else { std::cout << Period; }
            std::cout << ", DataType=" << KITFOX_DATA_STR[DataType] << std::endl;
#endif
            
            // Send a request.
            MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,rank,id,*INTER_COMM);
            
            // Receive a response.
            // This is blocking until the KitFox response returns.
            recv_buffer_position = 0;
            MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,rank,id,*INTER_COMM,&mpi_status);
            unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
            
            if(recv_message_type != KITFOX_MSG_TAG_SYNCHRONIZE_DATA_RES) {
                std::cout << "KITFOX ERROR (client) [ID=" << id << "]: received an unexpected message (" << KITFOX_MSG_TAG_STR[recv_message_type] << ")" << std::endl;
                exit(EXIT_FAILURE);
            }
            
            int queue_error;
            unpack_message(&queue_error,recv_buffer,sizeof(int),&recv_buffer_position);
            return queue_error;
        }*/
        
        const int synchronize_and_pull_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, void *Data)
        {
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_SYNCHRONIZE_AND_PULL_DATA_REQ;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(&ComponentID,send_buffer,sizeof(Comp_ID),&send_buffer_position);
            pack_message(&Time,send_buffer,sizeof(Second),&send_buffer_position);
            pack_message(&Period,send_buffer,sizeof(Second),&send_buffer_position);
            pack_message(&DataType,send_buffer,sizeof(int),&send_buffer_position);
            
#ifdef KITFOX_MPI_DEBUG
            std::cout << "KITFOX MPI DEBUG (client) [ID=" << id << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << " : ComponentID=" << ComponentID << ", Time=" << Time << ", Period="; 
            if(Period==MAX_TIME) { std::cout << "MAX_TIME"; }
            else { std::cout << Period; } 
            std::cout << ", DataType=" << KITFOX_DATA_STR[DataType] << ") to KitFox server [LP=" << rank << "]" << std::endl;
#endif
            
            // Send a request.
            MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,rank,id,*INTER_COMM);
            
            // Receive a response.
            // This is blocking until the KitFox response returns.
            recv_buffer_position = 0;
            MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,rank,id,*INTER_COMM,&mpi_status);
            unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
            
            if(recv_message_type != KITFOX_MSG_TAG_SYNCHRONIZE_AND_PULL_DATA_RES) {
                std::cout << "KITFOX ERROR (client) [ID=" << id << "]: received an unexpected message (" << KITFOX_MSG_TAG_STR[recv_message_type] << ")" << std::endl;
                exit(EXIT_FAILURE);
            }
            
            int queue_error;
            unpack_message(Data,recv_buffer,KITFOX_DATA_SIZE[DataType],&recv_buffer_position);
            unpack_message(&queue_error,recv_buffer,sizeof(int),&recv_buffer_position);
            return queue_error;
        }
        
        const int synchronize_data(Comp_ID ComponentID, Second Time, Second Period, int DataType)
        {
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_SYNCHRONIZE_DATA_REQ;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(&ComponentID,send_buffer,sizeof(Comp_ID),&send_buffer_position);
            pack_message(&Time,send_buffer,sizeof(Second),&send_buffer_position);
            pack_message(&Period,send_buffer,sizeof(Second),&send_buffer_position);
            pack_message(&DataType,send_buffer,sizeof(int),&send_buffer_position);

#ifdef KITFOX_MPI_DEBUG
            std::cout << "KITFOX MPI DEBUG (client) [ID=" << id << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << " : ComponentID=" << ComponentID << ", Time=" << Time << ", Period="; 
            if(Period==MAX_TIME) { std::cout << "MAX_TIME"; }
            else { std::cout << Period; }
            std::cout << ", DataType=" << KITFOX_DATA_STR[DataType] << ") to KitFox server [LP=" << rank << "]" << std::endl;
#endif
            
            // Send a request.
            MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,rank,id,*INTER_COMM);
            
            // Receive a response.
            // This is blocking until the KitFox response returns.
            recv_buffer_position = 0;
            MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,rank,id,*INTER_COMM,&mpi_status);
            unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
            
            if(recv_message_type != KITFOX_MSG_TAG_SYNCHRONIZE_DATA_RES) {
                std::cout << "KITFOX ERROR (client) [ID=" << id << "]: received an unexpected message (" << KITFOX_MSG_TAG_STR[recv_message_type] << ")" << std::endl;
                exit(EXIT_FAILURE);
            }
            
            int queue_error;
            unpack_message(&queue_error,recv_buffer,sizeof(int),&recv_buffer_position);
            return queue_error;
        }
        
        const int pull_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, void *Data)
        {
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_PULL_DATA_REQ;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(&ComponentID,send_buffer,sizeof(Comp_ID),&send_buffer_position);
            pack_message(&Time,send_buffer,sizeof(Second),&send_buffer_position);
            pack_message(&Period,send_buffer,sizeof(Second),&send_buffer_position);
            pack_message(&DataType,send_buffer,sizeof(int),&send_buffer_position);

#ifdef KITFOX_MPI_DEBUG
            std::cout << "KITFOX MPI DEBUG (client) [ID=" << id << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << " : ComponentID=" << ComponentID << ", Time=" << Time << ", Period="; 
            if(Period==MAX_TIME) { std::cout << "MAX_TIME"; }
            else { std::cout << Period; } 
            std::cout << ", DataType=" << KITFOX_DATA_STR[DataType] << ") to KitFox server [LP=" << rank << "]" << std::endl;
#endif
            
            // Send a request.
            MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,rank,id,*INTER_COMM);
            
            // Receive a response.
            // This is blocking until the KitFox response returns.
            recv_buffer_position = 0;
            MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,rank,id,*INTER_COMM,&mpi_status);
            unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
            
            if(recv_message_type != KITFOX_MSG_TAG_PULL_DATA_RES) {
                std::cout << "KITFOX ERROR (client) [ID=" << id << "]: received an unexpected message (" << KITFOX_MSG_TAG_STR[recv_message_type] << ")" << std::endl;
                exit(EXIT_FAILURE);
            }
            
            int queue_error;
            unpack_message(Data,recv_buffer,KITFOX_DATA_SIZE[DataType],&recv_buffer_position);
            unpack_message(&queue_error,recv_buffer,sizeof(int),&recv_buffer_position);
            return queue_error;
        }
        
        const int push_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, void *Data)
        {
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_PUSH_DATA_REQ;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(&ComponentID,send_buffer,sizeof(Comp_ID),&send_buffer_position);
            pack_message(&Time,send_buffer,sizeof(Second),&send_buffer_position);
            pack_message(&Period,send_buffer,sizeof(Second),&send_buffer_position);
            pack_message(&DataType,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(Data,send_buffer,KITFOX_DATA_SIZE[DataType],&send_buffer_position);

#ifdef KITFOX_MPI_DEBUG
            std::cout << "KITFOX MPI DEBUG (client) [ID=" << id << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << " : ComponentID=" << ComponentID << ", Time=" << Time << ", Period="; 
            if(Period==MAX_TIME) { std::cout << "MAX_TIME"; }
            else { std::cout << Period; } 
            std::cout << ", DataType=" << KITFOX_DATA_STR[DataType] << ") to KitFox server [LP=" << rank << "]" << std::endl;
#endif
            
            // Send q request.
            MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,rank,id,*INTER_COMM);
            
            // Receive a response.
            // This is blocking until the KitFox response returns.
            recv_buffer_position = 0;
            MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,rank,id,*INTER_COMM,&mpi_status);
            unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
            
            if(recv_message_type != KITFOX_MSG_TAG_PUSH_DATA_RES) {
                std::cout << "KITFOX ERROR (client) [ID=" << id << "]: received an unexpected message (" << KITFOX_MSG_TAG_STR[recv_message_type] << ")" << std::endl;
                exit(EXIT_FAILURE);
            }

            int queue_error;
            unpack_message(&queue_error,recv_buffer,sizeof(int),&recv_buffer_position);
            return queue_error;
        }
        
        const int overwrite_data(Comp_ID ComponentID, Second Time, Second Period, int DataType, void *Data)
        {
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_OVERWRITE_DATA_REQ;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(&ComponentID,send_buffer,sizeof(Comp_ID),&send_buffer_position);
            pack_message(&Time,send_buffer,sizeof(Second),&send_buffer_position);
            pack_message(&Period,send_buffer,sizeof(Second),&send_buffer_position);
            pack_message(&DataType,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(Data,send_buffer,KITFOX_DATA_SIZE[DataType],&send_buffer_position);

#ifdef KITFOX_MPI_DEBUG
            std::cout << "KITFOX MPI DEBUG (client) [ID=" << id << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << " : ComponentID=" << ComponentID << ", Time=" << Time << ", Period=";  
            if(Period==MAX_TIME) { std::cout << "MAX_TIME"; }
            else { std::cout << Period; } 
            std::cout << ", DataType=" << KITFOX_DATA_STR[DataType] << ") to KitFox server [LP=" << rank << "]" << std::endl;
#endif
            
            // Send q request.
            MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,rank,id,*INTER_COMM);
            
            // Receive a response.
            // This is blocking until the KitFox response returns.
            recv_buffer_position = 0;
            MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,rank,id,*INTER_COMM,&mpi_status);
            unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
            
            if(recv_message_type != KITFOX_MSG_TAG_OVERWRITE_DATA_RES) {
                std::cout << "KITFOX ERROR (client) [ID=" << id << "]: received an unexpected message (" << KITFOX_MSG_TAG_STR[recv_message_type] << ")" << std::endl;
                exit(EXIT_FAILURE);
            }

            int queue_error;
            unpack_message(&queue_error,recv_buffer,sizeof(int),&recv_buffer_position);
            return queue_error;
        }
        
        void calculate_power(Comp_ID ComponentID, Second Time, Second Period, counter_t Counter, bool IsTDP = false, bool Wait = false)
        {
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
            std::cout << "KITFOX MPI DEBUG (client) [ID=" << id << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << " : ComponentID=" << ComponentID << ", Time=" << Time << ", Period=" << Period << ", IsTDP=" << (const char*)(IsTDP?"True":"False") << ") to KitFox server [LP=" << rank << "]" << std::endl;
#endif
            
            // Send a request.
            MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,rank,id,*INTER_COMM);
            
            if(Wait) { // Wait for a response
                recv_buffer_position = 0;
                MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,rank,id,*INTER_COMM,&mpi_status);
                unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
            
                if(recv_message_type != KITFOX_MSG_TAG_CALCULATE_POWER_RES) {
                    std::cout << "KITFOX ERROR (client) [ID=" << id << "]: received an unexpected message (" << KITFOX_MSG_TAG_STR[recv_message_type] << ")" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
        }
        
        void calculate_temperature(Comp_ID ComponentID, Second Time, Second Period, bool DoPowerSync = true, bool Wait = false)
        {
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_CALCULATE_TEMPERATURE_REQ;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(&ComponentID,send_buffer,sizeof(Comp_ID),&send_buffer_position);
            pack_message(&Time,send_buffer,sizeof(Second),&send_buffer_position);
            pack_message(&Period,send_buffer,sizeof(Second),&send_buffer_position);
            pack_message(&DoPowerSync,send_buffer,sizeof(bool),&send_buffer_position);
            pack_message(&Wait,send_buffer,sizeof(bool),&send_buffer_position);
            
#ifdef KITFOX_MPI_DEBUG
            std::cout << "KITFOX MPI DEBUG (client) [ID=" << id << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << " : ComponentID=" << ComponentID << ", Time=" << Time << ", Period=" << Period << ", DoPowerSync=" << (const char*)(DoPowerSync?"True":"False") << ") to KitFox server [LP=" << rank << "]" << std::endl;
#endif            

            // Send a request.
            MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,rank,id,*INTER_COMM);
            
            if(Wait) { // Wait for a response
                recv_buffer_position = 0;
                MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,rank,id,*INTER_COMM,&mpi_status);
                unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
            
                if(recv_message_type != KITFOX_MSG_TAG_CALCULATE_TEMPERATURE_RES) {
                    std::cout << "KITFOX ERROR (client) [ID=" << id << "]: received an unexpected message (" << KITFOX_MSG_TAG_STR[recv_message_type] << ")" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
        }
        
        void calculate_failure_rate(Comp_ID ComponentID, Second Time, Second Period, bool DoDataSync = true, bool Wait = false)
        {
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_CALCULATE_FAILURE_RATE_REQ;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(&ComponentID,send_buffer,sizeof(Comp_ID),&send_buffer_position);
            pack_message(&Time,send_buffer,sizeof(Second),&send_buffer_position);
            pack_message(&Period,send_buffer,sizeof(Second),&send_buffer_position);
            pack_message(&DoDataSync,send_buffer,sizeof(bool),&send_buffer_position);
            pack_message(&Wait,send_buffer,sizeof(bool),&send_buffer_position);
            
#ifdef KITFOX_MPI_DEBUG
            std::cout << "KITFOX MPI DEBUG (client) [ID=" << id << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << " : ComponentID=" << ComponentID << ", Time=" << Time << ", Period=" << Period << ", DoDataSync=" << (const char*)(DoDataSync?"True":"False") << ") to KitFox server [LP=" << rank << "]" << std::endl;
#endif            

            // Send a request.
            MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,rank,id,*INTER_COMM);
            
            if(Wait) { // Wait for a response
                recv_buffer_position = 0;
                MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,rank,id,*INTER_COMM,&mpi_status);
                unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
            
                if(recv_message_type != KITFOX_MSG_TAG_CALCULATE_FAILURE_RATE_RES) {
                    std::cout << "KITFOX ERROR (client) [ID=" << id << "]: received an unexpected message (" << KITFOX_MSG_TAG_STR[recv_message_type] << ")" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
        }
        
        void update_library_variable(Comp_ID ComponentID, int VariableType, int VariableSize, void *Value, bool IsLibraryVariable=false, bool Wait=false)
        {
            send_buffer_position = 0;
            send_message_type = KITFOX_MSG_TAG_UPDATE_LIBRARY_VARIABLE_REQ;
            pack_message(&send_message_type,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(&ComponentID,send_buffer,sizeof(Comp_ID),&send_buffer_position);
            pack_message(&VariableType,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(&VariableSize,send_buffer,sizeof(int),&send_buffer_position);
            pack_message(Value,send_buffer,VariableSize,&send_buffer_position);
            pack_message(&IsLibraryVariable,send_buffer,sizeof(bool),&send_buffer_position);
            pack_message(&Wait,send_buffer,sizeof(bool),&send_buffer_position);
            
#ifdef KITFOX_MPI_DEBUG
            std::cout << "KITFOX MPI DEBUG (client) [ID=" << id << "]: MPI_Send(" << KITFOX_MSG_TAG_STR[send_message_type] << " : ComponentID=" << ComponentID << ", VariableType=";
            if(!IsLibraryVariable) std::cout << KITFOX_DATA_STR[VariableType];
            else std::cout << VariableType;
            std::cout << ", VariableSize=" << VariableSize << ", IsLibraryVariable=" << (const char*)(IsLibraryVariable?"True":"False") << ") top KitFox server [LP=" << rank << "]" << std::endl;
#endif
            
            // Send a request.
            MPI_Send(send_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_BYTE,rank,id,*INTER_COMM);
            
            if(Wait) { // Wait for a response
                recv_buffer_position = 0;
                MPI_Recv(recv_buffer,KITFOX_MPI_BUFFER_SIZE,MPI_PACKED,rank,id,*INTER_COMM,&mpi_status);
                unpack_message(&recv_message_type,recv_buffer,sizeof(int),&recv_buffer_position);
                
                if(recv_message_type != KITFOX_MSG_TAG_UPDATE_LIBRARY_VARIABLE_RES) {
                    std::cout << "KITFOX ERROR (client) [ID=" << id << "]: received an unexpected message (" << KITFOX_MSG_TAG_STR[recv_message_type] << ")" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
        }
        
    private:
        // Send/recv buffers
        uint8_t *send_buffer;
        uint8_t *recv_buffer;
        
        // Message and buffer controls
        int send_buffer_position;
        int recv_buffer_position;
        int send_message_type;
        int recv_message_type;
        
        // KitFox MPI communicator and varaibles
        MPI_Comm *INTER_COMM;
        MPI_Request mpi_request;
        MPI_Status mpi_status;
        int rank, id;
    };
} // namespace libKitFox

#endif

