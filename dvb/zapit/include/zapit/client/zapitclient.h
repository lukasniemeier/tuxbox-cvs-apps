/*
  Client-Interface f�r zapit  -   DBoxII-Project

  License: GPL

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __zapitclient__
#define __zapitclient__


#include <string>
#include <vector>

/* zapit */
#include "zapittypes.h"
#include "basicclient.h"


class CZapitClient:public CBasicClient
{
	inline bool zapit_connect();
	inline void zapit_close();

 private:
	void send(const unsigned char command, char* data, const unsigned int size);


 public:
	enum events
		{
			EVT_ZAP_FAILED,
			EVT_ZAP_COMPLETE,
			EVT_ZAP_COMPLETE_IS_NVOD,
			EVT_ZAP_SUB_COMPLETE,
			EVT_ZAP_SUB_FAILED,
			EVT_SCAN_COMPLETE,
			EVT_SCAN_NUM_TRANSPONDERS,
			EVT_SCAN_SATELLITE,
			EVT_SCAN_NUM_CHANNELS,
			EVT_SCAN_PROVIDER,
			EVT_RECORDMODE_ACTIVATED,
			EVT_RECORDMODE_DEACTIVATED,
			EVT_BOUQUETS_CHANGED
		};

	enum zapStatus
		{
			ZAP_OK = 0x01,
			ZAP_IS_NVOD = 0x02,
			ZAP_INVALID_PARAM = 0x04
		};

	enum bouquetMode
		{
			BM_CREATEBOUQUETS,
			BM_DELETEBOUQUETS,
			BM_DONTTOUCHBOUQUETS,
			BM_UPDATEBOUQUETS	 // not yet supported
		};


	typedef enum channelsMode_
		{
			MODE_CURRENT,
			MODE_TV,
			MODE_RADIO
		} channelsMode;

	typedef enum channelsOrder_
		{
			SORT_ALPHA,
			SORT_BOUQUET
		} channelsOrder;


	struct commandAddSubServices
	{
		t_original_network_id original_network_id;
		t_service_id          service_id;
		t_transport_stream_id transport_stream_id;
	};


	struct commandSetScanSatelliteList
	{
		char satName[30];
		int  diseqc;
	};
	typedef std::vector<commandSetScanSatelliteList> ScanSatelliteList;

	typedef std::vector<commandAddSubServices> subServiceList;



	struct responseGetLastChannel
	{
		unsigned int	channelNumber;
		char		mode;
	};

	struct responseGetBouquets
	{
		unsigned int bouquet_nr;
		char	 name[30];
		bool	 locked;
		bool	 hidden;
	};

	typedef std::vector<responseGetBouquets> BouquetList;

	struct responseChannels
	{
		unsigned int nr;
		t_channel_id channel_id;
		char	 name[30];
	};

	struct responseGetBouquetChannels : public responseChannels
	{};

	typedef std::vector<responseGetBouquetChannels> BouquetChannelList;

	struct responseGetAPIDs
	{
		uint    pid;
		char    desc[25];
		int     is_ac3;
		int     component_tag;
	};

	typedef std::vector<responseGetAPIDs> APIDList;

	struct responseGetOtherPIDs
	{
		uint		vpid;
		uint		ecmpid;
		uint		vtxtpid;
		uint		pcrpid;
		uint		selected_apid;
	};

	class CCurrentServiceInfo
		{
		public:
			t_original_network_id onid;
			t_service_id           sid;
			t_transport_stream_id tsid;
			unsigned short	vdid;
			unsigned short	apid;
			unsigned short	pcrpid;
			unsigned short	vtxtpid;
			unsigned int	tsfrequency;
			unsigned char	polarisation;
			unsigned char	diseqc;
		};

	struct responseGetPIDs
	{
		responseGetOtherPIDs	PIDs;
		APIDList		APIDs;
	};

	struct responseGetSatelliteList
	{
		char satName[30];
	};
	typedef std::vector<responseGetSatelliteList> SatelliteList;

 public:

	/****************************************/
	/*					*/
	/* general functions for zapping	*/
	/*					*/
	/****************************************/

	/* zaps to channel of specifeid bouquet */
	void zapTo( unsigned int bouquet, unsigned int channel );

	/* zaps to channel  */
	void zapTo( unsigned int channel );

	/* zaps to channel, returns the "zap-status" */
	unsigned int zapTo_serviceID(const t_channel_id channel_id);

	/* zaps to subservice, returns the "zap-status" */
	unsigned int zapTo_subServiceID(const t_channel_id channel_id);

	/* zaps to channel, does NOT wait for completion (uses event) */
	void zapTo_serviceID_NOWAIT(const t_channel_id channel_id);

	/* zaps to subservice, does NOT wait for completion (uses event) */
	void zapTo_subServiceID_NOWAIT(const t_channel_id channel_id);

	/* return the current (tuned) ServiceID */
	t_channel_id getCurrentServiceID();

	/* get last channel-information */
	void getLastChannel(unsigned int &channumber, char &mode);

	/* audiochan set */
	void setAudioChannel( unsigned channel );

	/* gets all bouquets */
	void getBouquets( BouquetList& bouquets, bool emptyBouquetsToo = false);

	/* gets all channels that are in specified bouquet */
	void getBouquetChannels( unsigned int bouquet, BouquetChannelList& channels, channelsMode mode = MODE_CURRENT);

	/* gets all channels */
	void getChannels( BouquetChannelList& channels, channelsMode mode = MODE_CURRENT, channelsOrder order = SORT_BOUQUET);

	/* restore bouquets so as if they where just loaded*/
	void restoreBouquets();

	/* reloads channels and services*/
	void reinitChannels();

	/* get current APID-List */
	void getPIDS( responseGetPIDs& pids );

	/* get info about the current serivice */
	CZapitClient::CCurrentServiceInfo getCurrentServiceInfo();

	/* transfer SubService-List to zapit */
	void setSubServices( subServiceList& subServices );

	/* set Mode */
	void setMode( channelsMode mode );

	/* set RecordMode*/
	void setRecordMode( bool activate );

	/* get RecordMode*/
	bool isRecordModeActive();

	/* mute audio */
	void muteAudio( bool mute );

	/* set audio volume */
	void setVolume( unsigned int left, unsigned int right );


	/****************************************/
	/*					*/
	/* Scanning stuff			*/
	/*					*/
	/****************************************/
	/* start TS-Scan */
	bool startScan();

	/* query if ts-scan is ready - response gives status */
	bool isScanReady(unsigned int &satellite, unsigned int &transponder, unsigned int &services );

	/* query possible satellits*/
	void getScanSatelliteList( SatelliteList& satelliteList );

	/* tell zapit which satellites to scan*/
	void setScanSatelliteList( ScanSatelliteList& satelliteList );

	/* set diseqcType*/
	void setDiseqcType( diseqc_t diseqc);

	/* set diseqcRepeat*/
	void setDiseqcRepeat( uint32_t repeat);

	/* set diseqcRepeat*/
	void setScanBouquetMode( bouquetMode mode);

	/****************************************/
	/*					*/
	/* Bouquet editing functions		*/
	/*					*/
	/****************************************/

	/* adds bouquet at the end of the bouquetlist*/
	void addBouquet(std::string name);

	/* moves a bouquet from one position to another, bouquet list begins at position=1*/
	void moveBouquet( unsigned int bouquet, unsigned int newPos);

	/* moves a channel of a bouquet from one position to another, channel lists begin at position=1*/
	void moveChannel( unsigned int bouquet, unsigned int oldPos, unsigned int newPos, channelsMode mode = MODE_CURRENT);

	/* deletes a bouquet with all its channels*/
	void deleteBouquet( unsigned int bouquet);

	/* assigns new name to bouquet*/
	void renameBouquet( unsigned int bouquet, std::string newName);

	// -- check if Bouquet-Name exists (2002-04-02 rasc)
	// -- Return bq_id or 0
	unsigned int  existsBouquet(std::string name);


	// -- check if Channel already in Bouquet (2002-04-05 rasc)
	// -- Return true/false
	bool  existsChannelInBouquet(const unsigned int bouquet, const t_channel_id channel_id);


	/* adds a channel at the end of then channel list to specified bouquet */
	/* same channels can be in more than one bouquet */
	/* bouquets can contain both tv and radio channels */
	void addChannelToBouquet(const unsigned int bouquet, const t_channel_id channel_id);

	/* removes a channel from specified bouquet */
	void removeChannelFromBouquet(const unsigned int bouquet, const t_channel_id channel_id);

	/* set a bouquet's lock-state*/
	void setBouquetLock( unsigned int bouquet, bool lock);

	/* set a bouquet's hidden-state*/
	void setBouquetHidden( unsigned int bouquet, bool hidden);

	/* renums the channellist, means gives the channels new numbers */
	/* based on the bouquet order and their order within bouquets */
	/* necessarily after bouquet editing operations*/
	void renumChannellist();

	/* saves current bouquet configuration to bouquets.xml*/
	void saveBouquets();

	/* commit bouquet change */
	void commitBouquetChange();

	/****************************************/
	/*					*/
	/* blah functions			*/
	/*					*/
	/****************************************/

	void startPlayBack();
	void stopPlayBack();
	bool isPlayBackActive();
	void setDisplayFormat(int mode);
	void setAudioMode(int mode);

	/****************************************/
	/*					*/
	/* Event functions			*/
	/*					*/
	/****************************************/

	/*
	  ein beliebiges Event anmelden
	*/
	void registerEvent(unsigned int eventID, unsigned int clientID, std::string udsName);

	/*
	  ein beliebiges Event abmelden
	*/
	void unRegisterEvent(unsigned int eventID, unsigned int clientID);

};



#endif
