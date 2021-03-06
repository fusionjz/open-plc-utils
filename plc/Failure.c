/*====================================================================*
 *
 *   Copyright (c) 2013 Qualcomm Atheros, Inc.
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or
 *   without modification, are permitted (subject to the limitations
 *   in the disclaimer below) provided that the following conditions
 *   are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials
 *     provided with the distribution.
 *
 *   * Neither the name of Qualcomm Atheros nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.
 *
 *   NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 *   GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE
 *   COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 *   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 *   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *
 *   void Failure (struct plc * plc, char const *format, ...)
 *
 *   error.h
 *
 *   Inform the user that an operation failed; print the channel name,
 *   source device, error message and user defined message on stderr
 *   unless the PLC_SILENCE flags is set;
 *
 *   This function is similar to Confirm () except that the message
 *   status or result code and description is output when the code
 *   is non-zero; overtime, result codes have replaced status codes
 *   and so we look in different places for codes in some MMEs;
 *
 *   the status and result code fields appear at different offsets
 *   in different messages; consequently, check the MMTYPE to find
 *   field;
 *
 *
 *   Contributor(s);
 *      Charles Maier
 *
 *--------------------------------------------------------------------*/

#ifndef FAILURE_SOURCE
#define FAILURE_SOURCE

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "../plc/plc.h"
#include "../tools/error.h"
#include "../tools/flags.h"
#include "../tools/memory.h"
#include "../mme/mme.h"

#ifdef __GNUC__

__attribute__ ((format (printf, 2, 3)))

#endif

void Failure (struct plc * plc, char const *format, ...)

{
	if (_allclr (plc->flags, PLC_SILENCE))
	{
		char address [ETHER_ADDR_LEN * 3];
		struct channel * channel = (struct channel *)(plc->channel);
		struct message * message = (struct message *)(plc->message);
		struct __packed header_confirm
		{
			ethernet_hdr ethernet;
			qualcomm_hdr qualcomm;
			uint8_t MSTATUS;
		}
		* header = (struct header_confirm *)(message);
		hexdecode (header->ethernet.OSA, sizeof (header->ethernet.OSA), address, sizeof (address));
		fprintf (stderr, "%s %s ", channel->ifname, address);
		switch (LE16TOH (header->qualcomm.MMTYPE))
		{
		case VS_CONN_ADD | MMTYPE_CNF:
		case VS_CONN_MOD | MMTYPE_CNF:
		case VS_CONN_REL | MMTYPE_CNF:
		case VS_CONN_INFO | MMTYPE_CNF:
			{
				struct __packed header_confirm
				{
					struct ethernet_hdr ethernet;
					struct qualcomm_hdr qualcomm;
					uint32_t REQUEST;
					uint8_t MSTATUS;
				}
				* header = (struct header_confirm *)(message);
				if (header->MSTATUS)
				{
					fprintf (stderr, "%s (0x%02X): ", MMECode (header->qualcomm.MMTYPE, header->MSTATUS), header->MSTATUS);
				}
			}
			break;
		case VS_SELFTEST_RESULTS | MMTYPE_CNF:
		case VS_FORWARD_CONFIG | MMTYPE_CNF:
			{
				struct __packed header_confirm
				{
					struct ethernet_hdr ethernet;
					struct qualcomm_hdr qualcomm;
					uint8_t MVERSION;
					uint8_t RESULTCODE;
				}
				* header = (struct header_confirm *)(message);
				if (header->RESULTCODE)
				{
					fprintf (stderr, "%s (0x%02X): ", MMECode (header->qualcomm.MMTYPE, header->RESULTCODE), header->RESULTCODE);
				}
			}
			break;
		default:
			if (header->MSTATUS)
			{
				fprintf (stderr, "%s (0x%02X): ", MMECode (header->qualcomm.MMTYPE, header->MSTATUS), header->MSTATUS);
			}
			break;
		}
		if ((format) && (*format))
		{
			va_list arglist;
			va_start (arglist, format);
			vfprintf (stderr, format, arglist);
			va_end (arglist);
		}
		fprintf (stderr, "\n");
	}
	if (_anyset (plc->flags, PLC_BAILOUT))
	{
		if (_allclr (plc->flags, PLC_SILENCE))
		{
			error (1, 0, "Bailing Out!");
		}
		exit (1);
	}
	return;
}


#endif

