/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifndef _UDP_MSG_H
#define _UDP_MSG_H

#define MAX_COMPRESSED_BITS       4096
#define UDP_MSG_MAX_PLAYERS          4

#pragma pack(push, 1)

#include <assert.h>

struct UdpMsg
{
	 enum MsgType { 
      Invalid,
      SyncRequest,
      SyncReply,   
      Input,
      QualityReport,
      QualityReply,
      KeepAlive, 
      InputAck,
	  Game_Client_Done_Connecting,
	  Game_Host_Says_Verify_Map,
	  Game_Client_Needs_Map,
	  Game_Client_Done_Verifying,
	  Game_Client_Has_Received_All_Data,
	  Game_Host_Says_Load,
	  Game_Client_Done_Loading,
	  Game_Host_Says_Start,
	  Game_Desync_Check,
	  Game_Map,
	  Game_Preview,
	  Game_Client_Needs_Preview,
	  Game_Host_Rematch,
	  Game_Client_Finished_Results_Screen,
	  Game_Host_Show_Post_Options,
   };
  /* enum MsgType { 
      Invalid       = 0,
      SyncRequest   = 1,
      SyncReply     = 2,
      Input         = 3,
      QualityReport = 4,
      QualityReply  = 5,
      KeepAlive     = 6,
      InputAck      = 7,
	  Game_Done_Loading 
	  Game_Load		= 8,
	  Game_Start	= 9,
   };*/

   struct connect_status {
      unsigned int   disconnected:1;
      int            last_frame:31;
   };

   struct {
      uint16         magic;
      uint16         sequence_number;
      uint8          type;            /* packet type */
   } hdr;
   union {
      struct {
         uint32      random_request;  /* please reply back with this random data */
         uint16      remote_magic;
         uint8       remote_endpoint;
      } sync_request;
      
      struct {
         uint32      random_reply;    /* OK, here's your random data back */
      } sync_reply;
      
      struct {
         int8        frame_advantage; /* what's the other guy's frame advantage? */
         uint32      ping;
      } quality_report;
      
      struct {
         uint32      pong;
      } quality_reply;

      struct {
         connect_status    peer_connect_status[UDP_MSG_MAX_PLAYERS];

         uint32            start_frame;

         int               disconnect_requested:1;
         int               ack_frame:31;

         uint16            num_bits;
         uint8             input_size; // XXX: shouldn't be in every single packet!
         uint8             bits[MAX_COMPRESSED_BITS]; /* must be last */
      } input;

      struct {
         int               ack_frame:31;
      } input_ack;

	  struct
	  {
		  double x;
		  double y;
		  uint32 frame_number;
		  uint32 player_action;
		  uint32 player_action_frame;
		  uint32 health; //testing
	  } desync_info;

   } u;

public:
   int PacketSize() {
      return sizeof(hdr) + PayloadSize();
   }


   int PayloadSize() {
      int size;

      switch (hdr.type) {
      case SyncRequest:   return sizeof(u.sync_request);
      case SyncReply:     return sizeof(u.sync_reply);
      case QualityReport: return sizeof(u.quality_report);
      case QualityReply:  return sizeof(u.quality_reply);
      case InputAck:      return sizeof(u.input_ack);
      case KeepAlive:     return 0;
      case Input:
         size = (int)((char *)&u.input.bits - (char *)&u.input);
         size += (u.input.num_bits + 7) / 8;
         return size;

	  case Game_Client_Done_Connecting: return 0;
	  case Game_Host_Says_Verify_Map: return 0;
	  case Game_Client_Needs_Map: return 0;
	  case Game_Client_Done_Verifying: return 0;
	  case Game_Client_Has_Received_All_Data: return 0;
	  case Game_Host_Says_Load: return 0;
	  case Game_Client_Done_Loading: return 0;
	  case Game_Host_Says_Start: return 0;
	  case Game_Desync_Check: return sizeof(u.desync_info);
	  case Game_Map: return 0;
	  case Game_Preview: return 0;
	  case Game_Client_Needs_Preview: return 0;
	  case Game_Host_Rematch: return 0;
	  case Game_Client_Finished_Results_Screen: return 0;
	  case Game_Host_Show_Post_Options: return 0;
      }

      //assert(false);
      return 0;
   }

   bool IsGameMsg()
   {
	   return(hdr.type > InputAck);
   }

   UdpMsg(MsgType t) { hdr.type = (uint8)t; }
};

#pragma pack(pop)

#endif   

