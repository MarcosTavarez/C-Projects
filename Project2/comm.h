/**
   Message definitions for communication over the pipes.

   All messages will include an initialized instance of HEADER.
   If appropriate, the header will be followed by a set of bytes
   that contains a set of arbitrary data.  The number of bytes sent
   must match header.len_message

   Note that not all of the fields of HEADER will have meaning for
   every message type.  At minimum, header.type and len_message must
   have content.  location and len_buffer will only be applicable for
   some messages.

*/

// Possible types of messages
typedef enum  {WRITE_REQUEST = 0, READ_REQUEST, ACKNOWLEDGE,
	       DATA, SHUTDOWN, INIT_CONNECTION} MessageType;

// Message header
typedef struct
{
  MessageType type;   // Type of the message
  int len_message;    // Length of data sent in message after header
  int location;       // Location to read/write in file
  int len_buffer;     // Length of bytes to read/write in file
}HEADER;
