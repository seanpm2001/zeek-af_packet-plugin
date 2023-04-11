// See the file "COPYING" in the main distribution directory for copyright.

#ifndef IOSOURCE_PKTSRC_AF_PACKET_SOURCE_H
#define IOSOURCE_PKTSRC_AF_PACKET_SOURCE_H

extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <errno.h>          // errorno
#include <unistd.h>         // close()

#include <net/ethernet.h>      // ETH_P_ALL
#include <linux/if.h>          // ifreq
#include <linux/if_packet.h>   // AF_PACKET, etc.
#include <linux/sockios.h>     // SIOCSHWTSTAMP
#include <linux/net_tstamp.h>  // hwtstamp_config
#include <pcap.h>
}

#include "zeek/iosource/PktSrc.h"
#include "RX_Ring.h"

namespace zeek::iosource::pktsrc {

class AF_PacketSource : public zeek::iosource::PktSrc {
public:
	/**
	 * Constructor.
	 *
	 * path: Name of the interface to open (the AF_Packet source doesn't
	 * support reading from files).
	 *
	 * is_live: Must be true (the AF_Packet source doesn't support offline
	 * operation).
	 */
	AF_PacketSource(const std::string& path, bool is_live);

	/**
	 * Destructor.
	 */
	virtual ~AF_PacketSource();

	static PktSrc* InstantiateAF_Packet(const std::string& path, bool is_live);

protected:
	// PktSrc interface.
	virtual void Open();
	virtual void Close();
	virtual bool ExtractNextPacket(zeek::Packet* pkt);
	virtual void DoneWithPacket();
	virtual bool PrecompileFilter(int index, const std::string& filter);
	virtual bool SetFilter(int index);
	virtual void Statistics(Stats* stats);

private:
	Properties props;
	Stats stats;

	int current_filter;
	unsigned int num_discarded;
	int checksum_mode;

	int socket_fd;
	RX_Ring *rx_ring;
	struct pcap_pkthdr current_hdr;

	struct InterfaceInfo {
		int index = -1;
		int flags = 0;

		bool Valid () { return index >= 0; }
		bool IsUp() { return flags & IFF_UP; }
		bool IsLoopback() { return flags & IFF_LOOPBACK; };
	};

	InterfaceInfo GetInterfaceInfo(const std::string& path);
	bool BindInterface(const InterfaceInfo& info);
	bool EnablePromiscMode(const InterfaceInfo& info);
	bool ConfigureFanoutGroup(bool enabled, bool defrag);
	bool ConfigureHWTimestamping(bool enabled);
	uint32_t GetFanoutMode(bool defrag);
};

}

#endif
