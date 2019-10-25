
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <arpa/inet.h>

#include <stdarg.h>

#include "ptpCompact.h"
#include "ptpProtocol.h"
#include "ptpImplements.h"
#include "util.h"
#include "ptpConfig.h"

#include "muxPtp.h"


#define IFMT "\n\t\t"

#if PTPC_NOISE_DEBUG
static char *_text2str(struct PTPText *text)
{
	static struct static_ptp_text s;
	s.max_symbols = -1;
	static_ptp_text_copy(&s, text);
	return (char*)(s.text);
}

static char *_bin2str(Octet *data, int len)
{
	static char buf[BIN_BUF_SIZE];
	return bin2str_impl(data, len, buf, sizeof(buf));
}
#endif

char *muxPtpId2Str(MUX_PTP_ID *id)
{
	static char buf[64];
	
	unsigned char *ptr = id->id;
	snprintf(buf, sizeof(buf), "%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x",  ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7] );
	return buf;
}

static void muxPmcDataClear(void *_muxPtp)
{
	MuxPtpRuntime *muxPtp = (MuxPtpRuntime *)_muxPtp;
	EXT_ASSERT((muxPtp!=NULL), "MuxPtp is null");
	
	memset(muxPtp, 0, offsetof(MuxPtpRuntime, pmc) );

	struct PtpMgmtClient *pmc = (struct PtpMgmtClient *)muxPtp->pmc;
	EXT_ASSERT((pmc!=NULL), "PMC is null");

}

static int muxPmcDataError(void *_muxPtp, int errCode, const char* frmt,...)
{
	char	buf[256];
	MuxPtpRuntime *muxPtp = (MuxPtpRuntime *)_muxPtp;
	EXT_ASSERT((muxPtp!=NULL), "MuxPtp is null");
	
	muxPmcDataClear(_muxPtp);
	
	muxPtp->errCode = errCode;
	memset(buf, 0, sizeof(buf));

	va_list ap;
	va_start(ap, frmt);
	vsnprintf(buf, sizeof(buf), frmt, ap);
	va_end(ap);

	printf("'%s'", buf);
	snprintf(muxPtp->msg, sizeof(muxPtp->msg), "%s", buf);	

	return EXIT_SUCCESS;
}


static void muxPmcParse(void *_muxPtp, struct ptp_message *msg)
{
	int action;
	struct TLV *tlv;
	struct management_tlv *mgt;
	struct defaultDS *dds;
	struct parentDS *pds;
	struct time_status_np *tsn;
	struct portDS *p;
#if PTPC_NOISE_DEBUG
	struct tlv_extra *extra;
	struct management_tlv_datum *mtd;
	struct currentDS *cds;
	struct timePropertiesDS *tp;
	struct grandmaster_settings_np *gsn;
	struct mgmt_clock_description *cd;
	struct port_ds_np *pnp;

#endif

	FILE *fp = stdout;
	MuxPtpRuntime *muxPtp = (MuxPtpRuntime *)_muxPtp;
	EXT_ASSERT((muxPtp!=NULL), "MuxPtp is null");
	struct PtpMgmtClient *pmc = (struct PtpMgmtClient *)muxPtp->pmc;
	EXT_ASSERT((pmc!=NULL), "PMC is null");

	if (msg_type(msg) != MANAGEMENT)
	{
		return;
	}
	
	action = management_action(msg);
	if (action < GET || action > ACKNOWLEDGE)
	{
		return;
	}
#if PTPC_NOISE_DEBUG
	fprintf(fp, "\t%s seq %hu %s ", pid2str(&msg->header.sourcePortIdentity), msg->header.sequenceId, pmc_action_string(action));
#endif
	if (msg_tlv_count(msg) != 1)
	{
		goto out;
	}
#if PTPC_NOISE_DEBUG
	extra = TAILQ_FIRST(&msg->tlv_list);
#endif

	tlv = (struct TLV *) msg->management.suffix;
	if (tlv->type == TLV_MANAGEMENT)
	{
		fprintf(fp, "MANAGEMENT ");
	}
	else if (tlv->type == TLV_MANAGEMENT_ERROR_STATUS)
	{
		fprintf(fp, "MANAGEMENT_ERROR_STATUS ");
		goto out;
	}
	else
	{
		fprintf(fp, "unknown-tlv ");
		goto out;
	}
	
	mgt = (struct management_tlv *) msg->management.suffix;
	if (mgt->length == 2 && mgt->id != TLV_NULL_MANAGEMENT)
	{
		fprintf(fp, "empty-tlv ");
		goto out;
	}
	
	switch (mgt->id)
	{
		case TLV_CLOCK_DESCRIPTION:
#if PTPC_NOISE_DEBUG
			cd = &extra->cd;
			fprintf(fp, "CLOCK_DESCRIPTION "
				IFMT "clockType             0x%hx"
				IFMT "physicalLayerProtocol %s"
				IFMT "physicalAddress       %s"
				IFMT "protocolAddress       %hu %s",
				align16(cd->clockType),
				_text2str(cd->physicalLayerProtocol),
				_bin2str(cd->physicalAddress->address,
					align16(&cd->physicalAddress->length)),
				align16(&cd->protocolAddress->networkProtocol),
				portaddr2str(cd->protocolAddress));
			fprintf(fp, IFMT "manufacturerId        %s"
				IFMT "productDescription    %s",
				_bin2str(cd->manufacturerIdentity, OUI_LEN),
				_text2str(cd->productDescription));
			fprintf(fp, IFMT "revisionData          %s",
				_text2str(cd->revisionData));
			fprintf(fp, IFMT "userDescription       %s"
				IFMT "profileId             %s",
				_text2str(cd->userDescription),
				_bin2str(cd->profileIdentity, PROFILE_ID_LEN));
#endif
			break;
			
		case TLV_USER_DESCRIPTION:
#if PTPC_NOISE_DEBUG
			fprintf(fp, "USER_DESCRIPTION "
				IFMT "userDescription  %s",
				_text2str(extra->cd.userDescription));
#endif
			break;
			
		case TLV_DEFAULT_DATA_SET:
			dds = (struct defaultDS *) mgt->data;
#if PTPC_NOISE_DEBUG
			fprintf(fp, "DEFAULT_DATA_SET "
				IFMT "twoStepFlag             %d"
				IFMT "slaveOnly               %d"
				IFMT "numberPorts             %hu"
				IFMT "priority1               %hhu"
				IFMT "clockClass              %hhu"
				IFMT "clockAccuracy           0x%02hhx"
				IFMT "offsetScaledLogVariance 0x%04hx"
				IFMT "priority2               %hhu"
				IFMT "clockIdentity           %s"
				IFMT "domainNumber            %hhu",
				dds->flags & DDS_TWO_STEP_FLAG ? 1 : 0,
				dds->flags & DDS_SLAVE_ONLY ? 1 : 0,
				dds->numberPorts,
				dds->priority1,
				dds->clockQuality.clockClass,
				dds->clockQuality.clockAccuracy,
				dds->clockQuality.offsetScaledLogVariance,
				dds->priority2,
				cid2str(&dds->clockIdentity),
				dds->domainNumber);
#endif

			muxPtp->isTwoStep = dds->flags & DDS_TWO_STEP_FLAG ? EXT_TRUE : EXT_FALSE;
			muxPtp->isSlaveOnly = dds->flags & DDS_SLAVE_ONLY ?EXT_TRUE : EXT_FALSE;
			muxPtp->priority1 = dds->priority1;
			muxPtp->clockClass = dds->clockQuality.clockClass;
			muxPtp->clockAccuracy = dds->clockQuality.clockAccuracy;
			muxPtp->offsetScaledLogVariance = dds->clockQuality.offsetScaledLogVariance;
			muxPtp->priority2 = dds->priority2;
			muxPtp->domain = dds->domainNumber;
			
			memcpy(muxPtp->clockId.id, dds->clockIdentity.id, 8);

			break;
			
		case TLV_CURRENT_DATA_SET:
#if PTPC_NOISE_DEBUG
			cds = (struct currentDS *) mgt->data;
			fprintf(fp, "CURRENT_DATA_SET "
				IFMT "stepsRemoved     %hd"
				IFMT "offsetFromMaster %.1f"
				IFMT "meanPathDelay    %.1f",
				cds->stepsRemoved, cds->offsetFromMaster / 65536.0,
				cds->meanPathDelay / 65536.0);
#endif
			break;
			
		case TLV_PARENT_DATA_SET:
			pds = (struct parentDS *) mgt->data;
#if PTPC_NOISE_DEBUG
			fprintf(fp, "PARENT_DATA_SET "
				IFMT "parentPortIdentity                    %s"
				IFMT "parentStats                           %hhu"
				IFMT "observedParentOffsetScaledLogVariance 0x%04hx"
				IFMT "observedParentClockPhaseChangeRate    0x%08x"
				IFMT "grandmasterPriority1                  %hhu"
				IFMT "gm.ClockClass                         %hhu"
				IFMT "gm.ClockAccuracy                      0x%02hhx"
				IFMT "gm.OffsetScaledLogVariance            0x%04hx"
				IFMT "grandmasterPriority2                  %hhu"
				IFMT "grandmasterIdentity                   %s",
				pid2str(&pds->parentPortIdentity),
				pds->parentStats,
				pds->observedParentOffsetScaledLogVariance,
				pds->observedParentClockPhaseChangeRate,
				pds->grandmasterPriority1,
				pds->grandmasterClockQuality.clockClass,
				pds->grandmasterClockQuality.clockAccuracy,
				pds->grandmasterClockQuality.offsetScaledLogVariance,
				pds->grandmasterPriority2,
				cid2str(&pds->grandmasterIdentity));
#endif

			muxPtp->gmPriority1 = pds->grandmasterPriority1;
			muxPtp->gmClockClass = pds->grandmasterClockQuality.clockClass;
			muxPtp->gmClockAccuracy = pds->grandmasterClockQuality.clockAccuracy;
			muxPtp->gmOffsetScaledLogVariance = pds->grandmasterClockQuality.offsetScaledLogVariance;
			muxPtp->gmPriority2 = pds->grandmasterPriority2;
			
			memcpy(muxPtp->sourceId.id, pds->parentPortIdentity.clockIdentity.id, 8);
			muxPtp->sourceIndex = pds->parentPortIdentity.portNumber;
			memcpy(muxPtp->masterId.id, pds->grandmasterIdentity.id, 8);
			
			break;
			
		case TLV_TIME_PROPERTIES_DATA_SET:
#if PTPC_NOISE_DEBUG
			tp = (struct timePropertiesDS *) mgt->data;
			fprintf(fp, "TIME_PROPERTIES_DATA_SET "
				IFMT "currentUtcOffset      %hd"
				IFMT "leap61                %d"
				IFMT "leap59                %d"
				IFMT "currentUtcOffsetValid %d"
				IFMT "ptpTimescale          %d"
				IFMT "timeTraceable         %d"
				IFMT "frequencyTraceable    %d"
				IFMT "timeSource            0x%02hhx",
				tp->currentUtcOffset,
				tp->flags & LEAP_61 ? 1 : 0,
				tp->flags & LEAP_59 ? 1 : 0,
				tp->flags & UTC_OFF_VALID ? 1 : 0,
				tp->flags & PTP_TIMESCALE ? 1 : 0,
				tp->flags & TIME_TRACEABLE ? 1 : 0,
				tp->flags & FREQ_TRACEABLE ? 1 : 0,
				tp->timeSource);
#endif
			break;
			
		case TLV_PRIORITY1:
#if PTPC_NOISE_DEBUG
			mtd = (struct management_tlv_datum *) mgt->data;
			fprintf(fp, "PRIORITY1 "
				IFMT "priority1 %hhu", mtd->val);
#endif
			break;
			
		case TLV_PRIORITY2:
#if PTPC_NOISE_DEBUG
			mtd = (struct management_tlv_datum *) mgt->data;
			fprintf(fp, "PRIORITY2 "
				IFMT "priority2 %hhu", mtd->val);
#endif
			break;
			
		case TLV_DOMAIN:
#if PTPC_NOISE_DEBUG
			mtd = (struct management_tlv_datum *) mgt->data;
			fprintf(fp, "DOMAIN "
				IFMT "domainNumber %hhu", mtd->val);
#endif
			break;
			
		case TLV_SLAVE_ONLY:
#if PTPC_NOISE_DEBUG
			mtd = (struct management_tlv_datum *) mgt->data;
			fprintf(fp, "SLAVE_ONLY "
				IFMT "slaveOnly %d", mtd->val & DDS_SLAVE_ONLY ? 1 : 0);
#endif
			break;
			
		case TLV_CLOCK_ACCURACY:
#if PTPC_NOISE_DEBUG
			mtd = (struct management_tlv_datum *) mgt->data;
			fprintf(fp, "CLOCK_ACCURACY "
				IFMT "clockAccuracy 0x%02hhx", mtd->val);
#endif
			break;
			
		case TLV_TRACEABILITY_PROPERTIES:
#if PTPC_NOISE_DEBUG
			mtd = (struct management_tlv_datum *) mgt->data;
			fprintf(fp, "TRACEABILITY_PROPERTIES "
				IFMT "timeTraceable      %d"
				IFMT "frequencyTraceable %d",
				mtd->val & TIME_TRACEABLE ? 1 : 0,
				mtd->val & FREQ_TRACEABLE ? 1 : 0);
#endif
			break;
			
		case TLV_TIMESCALE_PROPERTIES:
#if PTPC_NOISE_DEBUG
			mtd = (struct management_tlv_datum *) mgt->data;
			fprintf(fp, "TIMESCALE_PROPERTIES "
				IFMT "ptpTimescale %d", mtd->val & PTP_TIMESCALE ? 1 : 0);
#endif
			break;
			
		case TLV_TIME_STATUS_NP:
			tsn = (struct time_status_np *) mgt->data;
#if PTPC_NOISE_DEBUG
			fprintf(fp, "TIME_STATUS_NP "
				IFMT "master_offset              %" PRId64
				IFMT "ingress_time               %" PRId64
				IFMT "cumulativeScaledRateOffset %+.9f"
				IFMT "scaledLastGmPhaseChange    %d"
				IFMT "gmTimeBaseIndicator        %hu"
				IFMT "lastGmPhaseChange          0x%04hx'%016" PRIx64 ".%04hx"
				IFMT "gmPresent                  %s"
				IFMT "gmIdentity                 %s",
				tsn->master_offset,
				tsn->ingress_time,
				(tsn->cumulativeScaledRateOffset + 0.0) / P41,
				tsn->scaledLastGmPhaseChange,
				tsn->gmTimeBaseIndicator,
				tsn->lastGmPhaseChange.nanoseconds_msb,
				tsn->lastGmPhaseChange.nanoseconds_lsb,
				tsn->lastGmPhaseChange.fractional_nanoseconds,
				tsn->gmPresent ? "true" : "false",
				cid2str(&tsn->gmIdentity));
#endif

			/* initial muxPtp */
			muxPtp->masterPresent = (tsn->gmPresent)?EXT_TRUE:EXT_FALSE;
			memcpy(muxPtp->masterId.id, tsn->gmIdentity.id, 8);
			break;
			
		case TLV_GRANDMASTER_SETTINGS_NP:
#if PTPC_NOISE_DEBUG
			gsn = (struct grandmaster_settings_np *) mgt->data;
			fprintf(fp, "GRANDMASTER_SETTINGS_NP "
				IFMT "clockClass              %hhu"
				IFMT "clockAccuracy           0x%02hhx"
				IFMT "offsetScaledLogVariance 0x%04hx"
				IFMT "currentUtcOffset        %hd"
				IFMT "leap61                  %d"
				IFMT "leap59                  %d"
				IFMT "currentUtcOffsetValid   %d"
				IFMT "ptpTimescale            %d"
				IFMT "timeTraceable           %d"
				IFMT "frequencyTraceable      %d"
				IFMT "timeSource              0x%02hhx",
				gsn->clockQuality.clockClass,
				gsn->clockQuality.clockAccuracy,
				gsn->clockQuality.offsetScaledLogVariance,
				gsn->utc_offset,
				gsn->time_flags & LEAP_61 ? 1 : 0,
				gsn->time_flags & LEAP_59 ? 1 : 0,
				gsn->time_flags & UTC_OFF_VALID ? 1 : 0,
				gsn->time_flags & PTP_TIMESCALE ? 1 : 0,
				gsn->time_flags & TIME_TRACEABLE ? 1 : 0,
				gsn->time_flags & FREQ_TRACEABLE ? 1 : 0,
				gsn->time_source);
#endif
			break;
			
		case TLV_PORT_DATA_SET:
			p = (struct portDS *) mgt->data;
			if (p->portState > PS_SLAVE)
			{
				p->portState = 0;
			}
#if PTPC_NOISE_DEBUG
			fprintf(fp, "PORT_DATA_SET "
				IFMT "portIdentity            %s"
				IFMT "portState               %s"
				IFMT "logMinDelayReqInterval  %hhd"
				IFMT "peerMeanPathDelay       %" PRId64
				IFMT "logAnnounceInterval     %hhd"
				IFMT "announceReceiptTimeout  %hhu"
				IFMT "logSyncInterval         %hhd"
				IFMT "delayMechanism          %hhu"
				IFMT "logMinPdelayReqInterval %hhd"
				IFMT "versionNumber           %hhu",
				pid2str(&p->portIdentity), ptpPortStateString(p->portState),
				p->logMinDelayReqInterval, p->peerMeanPathDelay >> 16,
				p->logAnnounceInterval, p->announceReceiptTimeout,
				p->logSyncInterval, p->delayMechanism,
				p->logMinPdelayReqInterval, p->versionNumber);
#endif

			/* initial muxPtp */
			memcpy(muxPtp->portId.id, p->portIdentity.clockIdentity.id, 8);
			muxPtp->portState = p->portState;
			snprintf(muxPtp->portStateName, sizeof(muxPtp->portStateName), "%s", ptpPortStateString(p->portState));
			break;
			
		case TLV_PORT_DATA_SET_NP:
#if PTPC_NOISE_DEBUG
			pnp = (struct port_ds_np *) mgt->data;
			fprintf(fp, "PORT_DATA_SET_NP "
				IFMT "neighborPropDelayThresh %u"
				IFMT "asCapable               %d",
				pnp->neighborPropDelayThresh,
				pnp->asCapable ? 1 : 0);
#endif
			break;
		case TLV_LOG_ANNOUNCE_INTERVAL:
#if PTPC_NOISE_DEBUG
			mtd = (struct management_tlv_datum *) mgt->data;
			fprintf(fp, "LOG_ANNOUNCE_INTERVAL "
				IFMT "logAnnounceInterval %hhd", mtd->val);
#endif
			break;
		case TLV_ANNOUNCE_RECEIPT_TIMEOUT:
#if PTPC_NOISE_DEBUG
			mtd = (struct management_tlv_datum *) mgt->data;
			fprintf(fp, "ANNOUNCE_RECEIPT_TIMEOUT "
				IFMT "announceReceiptTimeout %hhu", mtd->val);
#endif
			break;
		case TLV_LOG_SYNC_INTERVAL:
#if PTPC_NOISE_DEBUG
			mtd = (struct management_tlv_datum *) mgt->data;
			fprintf(fp, "LOG_SYNC_INTERVAL "
				IFMT "logSyncInterval %hhd", mtd->val);
#endif
			break;
		case TLV_VERSION_NUMBER:
#if PTPC_NOISE_DEBUG
			mtd = (struct management_tlv_datum *) mgt->data;
			fprintf(fp, "VERSION_NUMBER "
				IFMT "versionNumber %hhu", mtd->val);
#endif
			break;
		case TLV_DELAY_MECHANISM:
#if PTPC_NOISE_DEBUG
			mtd = (struct management_tlv_datum *) mgt->data;
			fprintf(fp, "DELAY_MECHANISM "
				IFMT "delayMechanism %hhu", mtd->val);
#endif
			break;
		case TLV_LOG_MIN_PDELAY_REQ_INTERVAL:
#if PTPC_NOISE_DEBUG
			mtd = (struct management_tlv_datum *) mgt->data;
			fprintf(fp, "LOG_MIN_PDELAY_REQ_INTERVAL "
				IFMT "logMinPdelayReqInterval %hhd", mtd->val);
#endif
			break;
		}
out:
	fprintf(fp, "\n");
	fflush(fp);
}


static int muxPtpSearchOneCommand(void *_muxPtp, char *cmd)
{
	int cnt, tmo = -1;
	
#define N_FD		1

	struct pollfd pollfd[N_FD];
	char command[256];
	struct ptp_message *msg;
	
	MuxPtpRuntime *muxPtp = (MuxPtpRuntime *)_muxPtp;
	EXT_ASSERT((muxPtp!=NULL), "MuxPtp is null");
	struct PtpMgmtClient *pmc = (struct PtpMgmtClient *)muxPtp->pmc;
	EXT_ASSERT((pmc!=NULL), "PMC is null");
	int isSent = 0;
	

	pollfd[0].fd = pmc_get_transport_fd(pmc);

	snprintf(command, sizeof(command), "GET %s", cmd);
	EXT_INFOF("Cmd:%s, fd:%d", cmd, pollfd[0].fd);
//	while (is_running())
	while (isSent <= 1 )
	{
		/* No more commands, wait a bit for
		   any outstanding replies and exit. */
		tmo = 100;

		pollfd[0].events = POLLIN | POLLPRI;
		if( !isSent )
		{
			pollfd[0].events |= POLLOUT;
		}
		else
		{
			isSent ++;
		}

		cnt = poll(pollfd, N_FD, tmo);
		if (cnt < 0)
		{
			if (EINTR == errno)
			{
//				continue;
				EXT_INFOF("poll failed: %m");
				return EXIT_FAILURE;
			}
			else
			{
				pr_emerg("poll failed");
//				break;
				return EXIT_FAILURE;
			}
		}
		else if (!cnt)
		{
//			break;
			EXT_INFOF("No fd is available, domain %d", muxPtp->domainCfg);
			int domain = (int)muxPtp->domainCfg;
			
			muxPmcDataError(muxPtp, PTPC_ERROR_NO_CONNECTION, "No connection to PTP is available, domain %d, not correct", cmd, domain );
			return EXIT_FAILURE;
		}
		
		if (pollfd[0].revents & POLLHUP)
		{
			if (tmo == -1)
			{/* Wait a bit longer for outstanding replies. */
				tmo = 100;
//				pollfd[0].fd = -1;
				pollfd[0].events = 0;
			}
			else
			{
//				break;
				EXT_INFOF("Poll failed2");
				return EXIT_FAILURE;
			}
		}
		
		
		if (pollfd[0].revents & POLLOUT)
		{
			if (pmc_do_command(pmc, command))
			{
				EXT_ERRORF("bad command: %s", command);
			}
			//command = NULL;

			isSent = 1;
		}
		
		if (pollfd[0].revents & (POLLIN|POLLPRI))
		{
			msg = pmc_recv(pmc);
			if (msg)
			{
				muxPmcParse(muxPtp, msg);
				msg_put(msg);
			}
		}
		else
		{
			if(isSent > 1)
			{
				muxPmcDataError(muxPtp, PTPC_ERROR_NO_REPLY, "No response for management msg '%s' with domain number %d, check domain number", cmd, muxPtp->domainCfg);
				EXT_ERRORF("PMC Error: code: %d: %s", muxPtp->errCode, muxPtp->msg);
			}
		}
	}


	return EXIT_SUCCESS;
}

void muxPtpDestory(void *_muxPtp)
{
	MuxPtpRuntime *muxPtp = (MuxPtpRuntime *)_muxPtp;
	EXT_ASSERT((muxPtp!=NULL), "MuxPtp is null");

	struct PtpMgmtClient *pmc = (struct PtpMgmtClient *)muxPtp->pmc;
	EXT_ASSERT((pmc!=NULL), "PMC is null");

	pmc_destroy(pmc);
	msg_cleanup();

//	config_destroy(cfg);
}

void *muxPtpInit(void *_muxPtp, unsigned char domainNumber)
{
	const char *iface_name = NULL;
	int zero_datalen = 0;
	char uds_local[MAX_IFNAME_SIZE + 1];
	enum transport_type transport_type = TRANS_UDP_IPV4;
	UInteger8 boundary_hops = 1, domain_number = 0, transport_specific = 0;
	struct PtpConfig *cfg;

	struct PtpMgmtClient *_pmc = NULL;

	MuxPtpRuntime *muxPtp = (MuxPtpRuntime *)_muxPtp;
	EXT_ASSERT((muxPtp!=NULL), "MuxPtp is null");
//	handle_term_signals();

	cfg = config_create();
	if (!cfg)
	{
		EXT_ERRORF("failed to create config");
		return NULL;
	}


	if (config_set_int(cfg, "network_transport", TRANS_UDS))
	{
		EXT_ERRORF("failed to set network interface of pmc");
		goto out;
	}

	boundary_hops = 0;

	transport_type = config_get_int(cfg, NULL, "network_transport");
	transport_specific = config_get_int(cfg, NULL, "transportSpecific") << 4;
	domain_number = config_get_int(cfg, NULL, "domainNumber");

	domain_number = domainNumber;
	if (!iface_name)
	{
		if (transport_type == TRANS_UDS)
		{
			snprintf(uds_local, sizeof(uds_local), PTP_RUN_HOME"/pmc.%d", getpid());
			iface_name = uds_local;
		}
		else
		{
			iface_name = "eth0";
		}
	}
	

	print_set_progname("muxPtp");
	print_set_syslog(1);
	print_set_verbose(1);

	_pmc = pmc_create(cfg, transport_type, iface_name, boundary_hops, domain_number, transport_specific, zero_datalen);
	if (!_pmc)
	{
		EXT_ERRORF("failed to create pmc");
		goto out;
	}

//	config_destroy(cfg);
	muxPtp->pmc = _pmc;
	muxPtp->domainCfg = domain_number;
	EXT_INFOF("PTP Client: if:%s, domain: %d", iface_name, muxPtp->domainCfg);
	return _pmc;
	

out:
	config_destroy(cfg);
	return NULL;
}


int muxPtpRetrieve(void *_muxPtp)
{
	int ret = EXIT_SUCCESS;

	MuxPtpRuntime *muxPtp = (MuxPtpRuntime *)_muxPtp;
	EXT_ASSERT((muxPtp!=NULL), "MuxPtp is null");
	
	if(muxPtp->pmc == NULL )
	{
		EXT_ERRORF("pmc is null");
		ret = EXIT_FAILURE;
		goto out;
	}

	muxPmcDataClear(muxPtp);

	/* port ID, port status, status name */
	if(muxPtpSearchOneCommand(muxPtp, "PORT_DATA_SET") == EXIT_FAILURE)
	{
		EXT_ERRORF("PORT_DATA_SET is null");
		ret = EXIT_FAILURE;
		goto out;
	}

	/* gmPresent and gmIdentity */
	if(muxPtpSearchOneCommand(muxPtp, "TIME_STATUS_NP") == EXIT_FAILURE)
	{
		EXT_ERRORF("TIME_STATUS_NP is null");
		ret = EXIT_FAILURE;
		goto out;
	}

	if(muxPtpSearchOneCommand(muxPtp, "DEFAULT_DATA_SET") == EXIT_FAILURE)
	{
		EXT_ERRORF("DEFAULT_DATA_SET is null");
		ret = EXIT_FAILURE;
		goto out;
	}


	if(muxPtpSearchOneCommand(muxPtp, "PARENT_DATA_SET") == EXIT_FAILURE)
	{
		EXT_ERRORF("PARENT_DATA_SET is null");
		ret = EXIT_FAILURE;
		goto out;
	}
	
out:
	return ret;
}

int muxPtpDebug(void *_muxPtp)
{
	int ret = EXIT_SUCCESS;

	MuxPtpRuntime *muxPtp = (MuxPtpRuntime *)_muxPtp;
	EXT_ASSERT((muxPtp!=NULL), "MuxPtp is null");

	if(muxPtp->errCode)
	{
		EXT_ERRORF("Error: code: %d; details: %s", muxPtp->errCode, muxPtp->msg );
		return EXIT_FAILURE;
	}


	EXT_INFOF("PTP Clock ID:%s; port:%s, state:%s(%d)", muxPtpId2Str(&muxPtp->clockId),  muxPtpId2Str(&muxPtp->portId), muxPtp->portStateName, muxPtp->portState);
	EXT_INFOF("Default DS:TwoStep:%d, SlaveOnly:%d, priority1:%d, ClockClass:%d, ClockAccuracy:0x%2x, offsetScaledLogVariance:0x%4x; Priority2:%d", 
		muxPtp->isTwoStep, muxPtp->isSlaveOnly, muxPtp->priority1, muxPtp->clockClass, muxPtp->clockAccuracy, muxPtp->offsetScaledLogVariance, muxPtp->priority2);

	EXT_INFOF("PTP Master ID:%s, present:%d; source port:%s", muxPtpId2Str(&muxPtp->masterId), muxPtp->masterPresent,  muxPtpId2Str(&muxPtp->sourceId));
	EXT_INFOF("Parent DS:priority1:%d, ClockClass:%d, ClockAccuracy:0x%2x, offsetScaledLogVariance:0x%4x; Priority2:%d", 
		muxPtp->gmPriority1, muxPtp->gmClockClass, muxPtp->gmClockAccuracy, muxPtp->gmOffsetScaledLogVariance, muxPtp->gmPriority2);

	return ret;
}

int muxPtpDefaultConfig(void *_muxPtp)
{
	int ret = EXIT_SUCCESS;

	MuxPtpRuntime *muxPtp = (MuxPtpRuntime *)_muxPtp;
	EXT_ASSERT((muxPtp!=NULL), "MuxPtp is null");
	muxPmcDataClear(muxPtp);

	muxPtp->isEnable = EXT_TRUE;
	muxPtp->domainCfg = 0;
	muxPtp->isSlaveOnly = EXT_TRUE;
	muxPtp->isTwoStep = EXT_TRUE;

	return ret;
}

