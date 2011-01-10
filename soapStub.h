/* soapStub.h
   Generated by gSOAP 2.8.0 from winceconfig.h
   Copyright(C) 2000-2010, Robert van Engelen, Genivia Inc. All Rights Reserved.
   The generated code is released under one of the following licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
*/

#ifndef soapStub_H
#define soapStub_H
#include "stdsoap2.h"

/******************************************************************************\
 *                                                                            *
 * Enumerations                                                               *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * Types with Custom Serializers                                              *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * Classes and Structs                                                        *
 *                                                                            *
\******************************************************************************/


#if 0 /* volatile type: do not declare here, declared elsewhere */

#endif

#ifndef SOAP_TYPE_ns__addResponse
#define SOAP_TYPE_ns__addResponse (14)
/* ns:addResponse */
struct ns__addResponse
{
public:
	double result;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* required element of type xsd:double */
};
#endif

#ifndef SOAP_TYPE_ns__add
#define SOAP_TYPE_ns__add (15)
/* ns:add */
struct ns__add
{
public:
	double a;	/* required element of type xsd:double */
	double b;	/* required element of type xsd:double */
};
#endif

#ifndef SOAP_TYPE_ns__winconfigResponse
#define SOAP_TYPE_ns__winconfigResponse (18)
/* ns:winconfigResponse */
struct ns__winconfigResponse
{
public:
	bool result;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* required element of type xsd-boolean */
};
#endif

#ifndef SOAP_TYPE_ns__winconfig
#define SOAP_TYPE_ns__winconfig (19)
/* ns:winconfig */
struct ns__winconfig
{
public:
	char *key;	/* optional element of type xsd:string */
	char *value;	/* optional element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_SOAP_ENV__Header
#define SOAP_TYPE_SOAP_ENV__Header (20)
/* SOAP Header: */
struct SOAP_ENV__Header
{
#ifdef WITH_NOEMPTYSTRUCT
private:
	char dummy;	/* dummy member to enable compilation */
#endif
};
#endif

#ifndef SOAP_TYPE_SOAP_ENV__Code
#define SOAP_TYPE_SOAP_ENV__Code (21)
/* SOAP Fault Code: */
struct SOAP_ENV__Code
{
public:
	char *SOAP_ENV__Value;	/* optional element of type xsd:QName */
	struct SOAP_ENV__Code *SOAP_ENV__Subcode;	/* optional element of type SOAP-ENV:Code */
};
#endif

#ifndef SOAP_TYPE_SOAP_ENV__Detail
#define SOAP_TYPE_SOAP_ENV__Detail (23)
/* SOAP-ENV:Detail */
struct SOAP_ENV__Detail
{
public:
	int __type;	/* any type of element <fault> (defined below) */
	void *fault;	/* transient */
	char *__any;
};
#endif

#ifndef SOAP_TYPE_SOAP_ENV__Reason
#define SOAP_TYPE_SOAP_ENV__Reason (26)
/* SOAP-ENV:Reason */
struct SOAP_ENV__Reason
{
public:
	char *SOAP_ENV__Text;	/* optional element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_SOAP_ENV__Fault
#define SOAP_TYPE_SOAP_ENV__Fault (27)
/* SOAP Fault: */
struct SOAP_ENV__Fault
{
public:
	char *faultcode;	/* optional element of type xsd:QName */
	char *faultstring;	/* optional element of type xsd:string */
	char *faultactor;	/* optional element of type xsd:string */
	struct SOAP_ENV__Detail *detail;	/* optional element of type SOAP-ENV:Detail */
	struct SOAP_ENV__Code *SOAP_ENV__Code;	/* optional element of type SOAP-ENV:Code */
	struct SOAP_ENV__Reason *SOAP_ENV__Reason;	/* optional element of type SOAP-ENV:Reason */
	char *SOAP_ENV__Node;	/* optional element of type xsd:string */
	char *SOAP_ENV__Role;	/* optional element of type xsd:string */
	struct SOAP_ENV__Detail *SOAP_ENV__Detail;	/* optional element of type SOAP-ENV:Detail */
};
#endif

/******************************************************************************\
 *                                                                            *
 * Typedefs                                                                   *
 *                                                                            *
\******************************************************************************/

#ifndef SOAP_TYPE__QName
#define SOAP_TYPE__QName (5)
typedef char *_QName;
#endif

#ifndef SOAP_TYPE__XML
#define SOAP_TYPE__XML (6)
typedef char *_XML;
#endif

#ifndef SOAP_TYPE_xsd__string
#define SOAP_TYPE_xsd__string (7)
typedef char *xsd__string;
#endif

#ifndef SOAP_TYPE_xsd_boolean
#define SOAP_TYPE_xsd_boolean (9)
typedef bool xsd_boolean;
#endif

#ifndef SOAP_TYPE_xsd__double
#define SOAP_TYPE_xsd__double (11)
typedef double xsd__double;
#endif


/******************************************************************************\
 *                                                                            *
 * Externals                                                                  *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * Server-Side Operations                                                     *
 *                                                                            *
\******************************************************************************/


SOAP_FMAC5 int SOAP_FMAC6 ns__add(struct soap*, double a, double b, double &result);

SOAP_FMAC5 int SOAP_FMAC6 ns__winconfig(struct soap*, char *key, char *value, bool &result);

/******************************************************************************\
 *                                                                            *
 * Server-Side Skeletons to Invoke Service Operations                         *
 *                                                                            *
\******************************************************************************/

SOAP_FMAC5 int SOAP_FMAC6 soap_serve(struct soap*);

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_request(struct soap*);

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_ns__add(struct soap*);

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_ns__winconfig(struct soap*);

/******************************************************************************\
 *                                                                            *
 * Client-Side Call Stubs                                                     *
 *                                                                            *
\******************************************************************************/


SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__add(struct soap *soap, const char *soap_endpoint, const char *soap_action, double a, double b, double &result);

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns__winconfig(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *key, char *value, bool &result);

#endif

/* End of soapStub.h */
