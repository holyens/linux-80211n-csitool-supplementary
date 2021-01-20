/*
 * (c) 2008-2011 Daniel Halperin <dhalperi@cs.washington.edu>
 */
#include <linux/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#include <argp.h>
#include <tx80211.h>
#include <tx80211_packet.h>

#include "util.h"

static void init_lorcon();

/***********
  参数解析
************/
static char            *ifname = "mon0";
static int             num_packets = 1;
static int             delay_us = 100000;
static char            *filepath = "pkts/default.pkt";
static int             log_level = 2;

const char *argp_program_version = "V20.12.26";
const char *argp_program_bug_address = "<shwei@tju.edu.cn>";

static char doc[] = "injector -- a program for frame injection.";

static struct argp_option options[] = {
	{"interface", 'I', "STRING", 0, "Set interface name (default: mon0)"},
	{"num", 'n', "INT", 0, "Set the number of packets will be send (default: 1)"},
    {"period", 'p', "INT", OPTION_ARG_OPTIONAL, "Set period (us) (default: 100000)"},
	{"file", 'f', "STRING", OPTION_ARG_OPTIONAL, "read a frame from file"},
	{"log-level", 'l', "INT", 0, "set log level (default: 2)"},
    { 0 }
};

static error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
    switch (key) {
        case 'I':
            ifname = arg;
            break;
        case 'n':
			num_packets = strtoul(arg, NULL, 10);
            break;
        case 'p':
			delay_us = strtoul(arg, NULL, 10);
            break;
		case 'f':
			filepath = arg;
            break;
		case 'l':
			log_level = strtol(arg, NULL, 0);
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}
static struct argp argp = { options, parse_opt, 0, doc };

#define log(level, format, ...) (void)(log_level>=level && printf(format, ##__VA_ARGS__))

struct tx80211	tx;
struct tx80211_packet	tx_packet;
#define PAYLOAD_SIZE	2000000

int readFile(uint8_t *buf, int max_size, const char* filepath, uint8_t file_type)
{
    int32_t fd = open(filepath, O_RDONLY);
    if(fd < 0){
        fprintf(stderr, "<main loop> open file \"%s\" failed!\n", filepath);
        return -1;
    }
    int read_size = read(fd, buf, max_size);
    if(read_size<0){
        fprintf(stderr, "<main> read file \"%s\" failed!\n", filepath);
        return -2;
    }
    close(fd);
    return read_size;
}

struct lorcon_packet
{
	__le16	fc;
	__le16	dur;
	u_char	addr1[6];
	u_char	addr2[6];
	u_char	addr3[6];
	__le16	seq;
	u_char	payload[0];
} __attribute__ ((packed));

int main(int argc, char** argv)
{
	uint8_t *packet_buf;
	uint32_t packet_size;
	uint32_t i;
	int32_t ret;
	struct timespec start, now;
	int32_t diff;

	argp_parse(&argp, argc, argv, 0, 0, 0);
	/* Generate packet payloads */
	printf("Generating packet payloads \n");
	packet_buf = malloc(PAYLOAD_SIZE);
	if (packet_buf == NULL) {
		perror("malloc payload buffer");
		exit(1);
	}

	// frame
    packet_size = readFile(packet_buf, PAYLOAD_SIZE, filepath, 0);
    if (packet_size<0)
        exit(-1);
    
	/* Setup the interface for lorcon */
	printf("Initializing LORCON\n");
	init_lorcon();
	// packet = (struct lorcon_packet*)payload_buffer;
	// packet->dur = 0xffff;
	// packet->seq = 0;
	tx_packet.packet = (uint8_t *)packet_buf;
	tx_packet.plen = packet_size;

	/* Send packets */
	printf("Sending %u packets of size %u (. every thousand)\n", num_packets, packet_size);
	if (delay_us) {
		/* Get start time */
		clock_gettime(CLOCK_MONOTONIC, &start);
	}
	for (i = 0; i < num_packets || !num_packets; ++i) {
		
		if (delay_us) {
			clock_gettime(CLOCK_MONOTONIC, &now);
			diff = (now.tv_sec - start.tv_sec) * 1000000 +
			       (now.tv_nsec - start.tv_nsec + 500) / 1000;
			diff = delay_us*i - diff;
			if (diff > 0 && diff < delay_us)
				usleep(diff);
		}

		ret = tx80211_txpacket(&tx, &tx_packet);
		if (ret < 0) {
			fprintf(stderr, "Unable to transmit packet: %s\n",
					tx.errstr);
			exit(1);
		}

		if (((i+1) % 1000) == 0) {
			printf(".");
			fflush(stdout);
		}
		if (((i+1) % 50000) == 0) {
			printf("%dk\n", (i+1)/1000);
			fflush(stdout);
		}
	}

	return 0;
}

static void init_lorcon()
{
	/* Parameters for LORCON */
	int drivertype = tx80211_resolvecard("iwlwifi");

	/* Initialize LORCON tx struct */
	if (tx80211_init(&tx, "mon0", drivertype) < 0) {
		fprintf(stderr, "Error initializing LORCON: %s\n",
				tx80211_geterrstr(&tx));
		exit(1);
	}
	if (tx80211_open(&tx) < 0 ) {
		fprintf(stderr, "Error opening LORCON interface\n");
		exit(1);
	}

	/* Set up rate selection packet */
	tx80211_initpacket(&tx_packet);
}

