/*
 * Copyright: JessMA Open Source (ldcsaa@gmail.com)
 *
 * Author	: Bruce Liang
 * Website	: https://github.com/ldcsaa
 * Project	: https://github.com/ldcsaa/HP-Socket
 * Blog		: http://www.cnblogs.com/ldcsaa
 * Wiki		: http://www.oschina.net/p/hp-socket
 * QQ Group	: 44636872, 75375912
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "../include/hpsocket/HPTypeDef.h"

/************************************************************************
���ƣ�ȫ�ֳ���
��������������Ĺ���ȫ�ֳ���
************************************************************************/

/* Socket ��������Сֵ */
#define MIN_SOCKET_BUFFER_SIZE					8
/* С�ļ�����ֽ��� */
#define MAX_SMALL_FILE_SIZE						0x3FFFFF
/* �������ʱ�� */
#define MAX_CONNECTION_PERIOD					(MAXINT / 2)
/* ���������¼�ʱ����ȡ���� */
#define MAX_CONTINUE_READS						30
/* ���������¼�ʱ���д����� */
#define MAX_CONTINUE_WRITES						50

/* Ĭ�Ϲ������еȴ�������������¼����� */
#define DEFAULT_WORKER_MAX_EVENT_COUNT			CIODispatcher::DEF_WORKER_MAX_EVENTS

/* Server/Agent ��������� */
#define MAX_CONNECTION_COUNT					(5 * 1000 * 1000)
/* Server/Agent Ĭ����������� */
#define DEFAULT_CONNECTION_COUNT				10000
/* Server/Agent Ĭ�� Socket ���󻺴�����ʱ�� */
#define DEFAULT_FREE_SOCKETOBJ_LOCK_TIME		DEFAULT_OBJECT_CACHE_LOCK_TIME
/* Server/Agent Ĭ�� Socket ����ش�С */
#define DEFAULT_FREE_SOCKETOBJ_POOL				DEFAULT_OBJECT_CACHE_POOL_SIZE
/* Server/Agent Ĭ�� Socket ����ػ��շ�ֵ */
#define DEFAULT_FREE_SOCKETOBJ_HOLD				DEFAULT_OBJECT_CACHE_POOL_HOLD
/* Server/Agent Ĭ���ڴ�黺��ش�С */
#define DEFAULT_FREE_BUFFEROBJ_POOL				DEFAULT_BUFFER_CACHE_POOL_SIZE
/* Server/Agent Ĭ���ڴ�黺��ػ��շ�ֵ */
#define DEFAULT_FREE_BUFFEROBJ_HOLD				DEFAULT_BUFFER_CACHE_POOL_HOLD
/* Client Ĭ���ڴ�黺��ش�С */
#define DEFAULT_CLIENT_FREE_BUFFER_POOL_SIZE	60
/* Client Ĭ���ڴ�黺��ػ��շ�ֵ */
#define DEFAULT_CLIENT_FREE_BUFFER_POOL_HOLD	60
/* IPv4 Ĭ�ϰ󶨵�ַ */
#define  DEFAULT_IPV4_BIND_ADDRESS				_T("0.0.0.0")
/* IPv6 Ĭ�ϰ󶨵�ַ */
#define  DEFAULT_IPV6_BIND_ADDRESS				_T("::")
/* IPv4 �㲥��ַ */
#define DEFAULT_IPV4_BROAD_CAST_ADDRESS			_T("255.255.255.255")

/* TCP Ĭ��ͨ�����ݻ�������С */
#define DEFAULT_TCP_SOCKET_BUFFER_SIZE			DEFAULT_BUFFER_CACHE_CAPACITY
/* TCP Ĭ����������� */
#define DEFALUT_TCP_KEEPALIVE_TIME				(60 * 1000)
/* TCP Ĭ������ȷ�ϰ������ */
#define DEFALUT_TCP_KEEPALIVE_INTERVAL			(20 * 1000)
/* TCP Server Ĭ�� Listen ���д�С */
#define DEFAULT_TCP_SERVER_SOCKET_LISTEN_QUEUE	SOMAXCONN

/* UDP ������ݱ�����󳤶� */
#define MAXIMUM_UDP_MAX_DATAGRAM_SIZE			(16 * DEFAULT_BUFFER_CACHE_CAPACITY)
/* UDP Ĭ�����ݱ�����󳤶� */
#define DEFAULT_UDP_MAX_DATAGRAM_SIZE			1432
/* UDP Ĭ�� Receive ԤͶ������ */
#define DEFAULT_UDP_POST_RECEIVE_COUNT			DEFAULT_WORKER_MAX_EVENT_COUNT
/* UDP Ĭ�ϼ������Դ��� */
#define DEFAULT_UDP_DETECT_ATTEMPTS				3
/* UDP Ĭ�ϼ������ͼ�� */
#define DEFAULT_UDP_DETECT_INTERVAL				(60 * 1000)

/* TCP Pack ������λ�� */
#define TCP_PACK_LENGTH_BITS					22
/* TCP Pack ���������� */
#define TCP_PACK_LENGTH_MASK					0x3FFFFF
/* TCP Pack ����󳤶�Ӳ���� */
#define TCP_PACK_MAX_SIZE_LIMIT					0x3FFFFF
/* TCP Pack ��Ĭ����󳤶� */
#define TCP_PACK_DEFAULT_MAX_SIZE				0x040000
/* TCP Pack ��ͷ��ʶֵӲ���� */
#define TCP_PACK_HEADER_FLAG_LIMIT				0x0003FF
/* TCP Pack ��ͷĬ�ϱ�ʶֵ */
#define TCP_PACK_DEFAULT_HEADER_FLAG			0x000000

#define PORT_SEPARATOR_CHAR						':'
#define IPV6_ADDR_BEGIN_CHAR					'['
#define IPV6_ADDR_END_CHAR						']'
#define IPV4_ADDR_SEPARATOR_CHAR				'.'
#define IPV6_ADDR_SEPARATOR_CHAR				':'
#define IPV6_ZONE_INDEX_CHAR					'%'

#define CST_CONNECTING							(-1)
#define INVALID_SOCKET							INVALID_FD
#define SOCKET_ERROR							HAS_ERROR
#define WSASetLastError							SetLastError
#define WSAGetLastError							GetLastError
#define InetPton								inet_pton
#define InetNtop								inet_ntop
#define closesocket								close

#define ENSURE_STOP()							{if(GetState() != SS_STOPPED) {Stop();} Wait();}
#define ENSURE_HAS_STOPPED()					{ASSERT(GetState() == SS_STOPPED); if(GetState() != SS_STOPPED) return;}
#define WAIT_FOR_STOP_PREDICATE					[this]() {return GetState() == SS_STOPPED;}