/*
             LUFA Library
     Copyright (C) Dean Camera, 2013.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2013  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#include <usb/core/usbh_config_descriptors.h>



void usb_get_next_descriptor_of_type(uint16_t* const BytesRem,
                                 const void** CurrConfigLoc,
                                 const uint8_t Type)
{
	while (*BytesRem)
	{
		usb_get_next_descriptor(BytesRem, CurrConfigLoc);

		if (DESCRIPTOR_TYPE(*CurrConfigLoc) == Type)
		  return;
	}
}

void usb_get_next_descriptor_of_type_before(uint16_t* const BytesRem,
                                       const void** CurrConfigLoc,
                                       const uint8_t Type,
                                       const uint8_t BeforeType)
{
	while (*BytesRem)
	{
		usb_get_next_descriptor(BytesRem, CurrConfigLoc);

		if (DESCRIPTOR_TYPE(*CurrConfigLoc) == Type)
		{
			return;
		}
		else if (DESCRIPTOR_TYPE(*CurrConfigLoc) == BeforeType)
		{
			*BytesRem = 0;
			return;
		}
	}
}

void usb_get_next_descriptor_of_type_after(uint16_t* const BytesRem,
                                      const void** CurrConfigLoc,
                                      const uint8_t Type,
                                      const uint8_t AfterType)
{
	usb_get_next_descriptor_of_type(BytesRem, CurrConfigLoc, AfterType);

	if (*BytesRem)
	  usb_get_next_descriptor_of_type(BytesRem, CurrConfigLoc, Type);
}

int usb_get_next_descriptor_comp(uint16_t* const BytesRem,
                                  const void** CurrConfigLoc,
                                  config_comparator_ptr_t const ComparatorRoutine)
{
	uint8_t ErrorCode;

	while (*BytesRem)
	{
		const uint8_t* PrevDescLoc  = *CurrConfigLoc;
		uint16_t PrevBytesRem = *BytesRem;

		usb_get_next_descriptor(BytesRem, CurrConfigLoc);

		if ((ErrorCode = ComparatorRoutine(*CurrConfigLoc)) != DESCRIPTOR_SEARCH_NotFound)
		{
			if (ErrorCode == DESCRIPTOR_SEARCH_Fail)
			{
				*CurrConfigLoc = PrevDescLoc;
				*BytesRem      = PrevBytesRem;
			}

			return ErrorCode;
		}
	}

	return DESCRIPTOR_SEARCH_COMP_EndOfDescriptor;
}

