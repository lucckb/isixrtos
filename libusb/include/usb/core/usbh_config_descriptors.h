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

/** \file
 *  \brief USB Configuration Descriptor definitions.
 *  \copydetails Group_ConfigDescriptorParser
 *
 *  \note This file should not be included directly. It is automatically included as needed by the USB driver
 *        dispatch header located in LUFA/Drivers/USB/USB.h.
 */

/** \ingroup Group_StdDescriptors
 *  \defgroup Group_ConfigDescriptorParser Configuration Descriptor Parser
 *  \brief USB Configuration Descriptor definitions.
 *
 *  This section of the library gives a friendly API which can be used in host applications to easily
 *  parse an attached device's configuration descriptor so that endpoint, interface and other descriptor
 *  data can be extracted and used as needed.
 *
 *  @{
 */
#pragma once

#include <stdint.h>

	/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			extern "C" {
		#endif


	/* Public Interface - May be used in end-application: */
		/* Macros: */
			/** Casts a pointer to a descriptor inside the configuration descriptor into a pointer to the given
			 *  descriptor type.
			 *
			 *  Usage Example:
			 *  \code
			 *  uint8_t* CurrDescriptor = &ConfigDescriptor[0]; // Pointing to the configuration header
			 *  USB_Descriptor_Configuration_Header_t* ConfigHeaderPtr = DESCRIPTOR_PCAST(CurrDescriptor,
			 *                                                           USB_Descriptor_Configuration_Header_t);
			 *
			 *  // Can now access elements of the configuration header struct using the -> indirection operator
			 *  \endcode
			 */
			#define DESCRIPTOR_PCAST(DescriptorPtr, Type) ((Type*)(DescriptorPtr))

			/** Casts a pointer to a descriptor inside the configuration descriptor into the given descriptor
			 *  type (as an actual struct instance rather than a pointer to a struct).
			 *
			 *  Usage Example:
			 *  \code
			 *  uint8_t* CurrDescriptor = &ConfigDescriptor[0]; // Pointing to the configuration header
			 *  USB_Descriptor_Configuration_Header_t ConfigHeader = DESCRIPTOR_CAST(CurrDescriptor,
			 *                                                       USB_Descriptor_Configuration_Header_t);
			 *
			 *  // Can now access elements of the configuration header struct using the . operator
			 *  \endcode
		 */
			#define DESCRIPTOR_CAST(DescriptorPtr, Type)  (*DESCRIPTOR_PCAST(DescriptorPtr, Type))

			/** Returns the descriptor's type, expressed as the 8-bit type value in the header of the descriptor.
			 *  This value's meaning depends on the descriptor's placement in the descriptor, but standard type
			 *  values can be accessed in the \ref USB_DescriptorTypes_t enum.
			 */
			#define DESCRIPTOR_TYPE(DescriptorPtr)    DESCRIPTOR_PCAST(DescriptorPtr, usb_descriptor_header_t)->Type

			/** Returns the descriptor's size, expressed as the 8-bit value indicating the number of bytes. */
			#define DESCRIPTOR_SIZE(DescriptorPtr)    DESCRIPTOR_PCAST(DescriptorPtr, usb_descriptor_header_t)->Size
				typedef struct
				{
					uint8_t Size; 
					uint8_t Type; 
				} __attribute__((packed)) usb_descriptor_header_t;
				/** Enum for return values of a descriptor comparator function. */
				enum DSearch_Return_ErrorCodes_t
				{
					DESCRIPTOR_SEARCH_Found                = 0, /**< Current descriptor matches comparator criteria. */
					DESCRIPTOR_SEARCH_Fail                 = 1, /**< No further descriptor could possibly match criteria, fail the search. */
					DESCRIPTOR_SEARCH_NotFound             = 2, /**< Current descriptor does not match comparator criteria. */
				};
				/** Enum for return values of \ref usb_get_next_descriptor_comp(). */
				enum DSearch_Comp_Return_ErrorCodes_t
				{
					DESCRIPTOR_SEARCH_COMP_Found           = 0, /**< Configuration descriptor now points to descriptor which matches
																*   search criteria of the given comparator function. */
					DESCRIPTOR_SEARCH_COMP_Fail            = 1, /**< Comparator function returned \ref DESCRIPTOR_SEARCH_Fail. */
					DESCRIPTOR_SEARCH_COMP_EndOfDescriptor = 2, /**< End of configuration descriptor reached before match found. */
				};
		/* Type Defines: */
			/** Type define for a Configuration Descriptor comparator function (function taking a pointer to an array
			 *  of type void, returning a uint8_t value).
			 *
			 *  \see \ref usb_get_next_descriptor_comp function for more details.
			 */
			typedef int (* config_comparator_ptr_t)(const void*);

			/** Skips to the next sub-descriptor inside the configuration descriptor of the specified type value.
			 *  The bytes remaining value is automatically decremented.
			 *
			 * \param[in,out] BytesRem       Pointer to the number of bytes remaining of the configuration descriptor.
			 * \param[in,out] CurrConfigLoc  Pointer to the current descriptor inside the configuration descriptor.
			 * \param[in]     Type           Descriptor type value to search for.
			 */
			void usb_get_next_descriptor_of_type(uint16_t* const BytesRem,
			                                 const void** CurrConfigLoc,
			                                 const uint8_t Type);

			/** Skips to the next sub-descriptor inside the configuration descriptor of the specified type value,
			 *  which must come before a descriptor of the second given type value. If the BeforeType type
			 *  descriptor is reached first, the number of bytes remaining to process is set to zero and the
			 *  function exits. The bytes remaining value is automatically decremented.
			 *
			 * \param[in,out] BytesRem       Pointer to the number of bytes remaining of the configuration descriptor.
			 * \param[in,out] CurrConfigLoc  Pointer to the current descriptor inside the configuration descriptor.
			 * \param[in]     Type           Descriptor type value to search for.
			 * \param[in]     BeforeType     Descriptor type value which must not be reached before the given Type descriptor.
			 */
			void usb_get_next_descriptor_of_type_before(uint16_t* const BytesRem,
			                                       const void** CurrConfigLoc,
			                                       const uint8_t Type,
			                                       const uint8_t BeforeType);

			/** Skips to the next sub-descriptor inside the configuration descriptor of the specified type value,
			 *  which must come after a descriptor of the second given type value. The bytes remaining value is
			 *  automatically decremented.
			 *
			 * \param[in,out] BytesRem       Pointer to the number of bytes remaining of the configuration descriptor.
			 * \param[in,out] CurrConfigLoc  Pointer to the current descriptor inside the configuration descriptor.
			 * \param[in]     Type           Descriptor type value to search for.
			 * \param[in]     AfterType      Descriptor type value which must be reached before the given Type descriptor.
			 */
			void usb_get_next_descriptor_of_type_after(uint16_t* const BytesRem,
			                                      const void** CurrConfigLoc,
			                                      const uint8_t Type,
			                                      const uint8_t AfterType);

			/** Searches for the next descriptor in the given configuration descriptor using a pre-made comparator
			 *  function. The routine updates the position and remaining configuration descriptor bytes values
			 *  automatically. If a comparator routine fails a search, the descriptor pointer is retreated back
			 *  so that the next descriptor search invocation will start from the descriptor which first caused the
			 *  original search to fail. This behavior allows for one comparator to be used immediately after another
			 *  has failed, starting the second search from the descriptor which failed the first.
			 *
			 *  Comparator functions should be standard functions which accept a pointer to the header of the current
			 *  descriptor inside the configuration descriptor which is being compared, and should return a value from
			 *  the \ref DSearch_Return_ErrorCodes_t enum as a uint8_t value.
			 *
			 *  \note This function is available in USB Host mode only.
			 *
			 *  \param[in,out] BytesRem           Pointer to an int storing the remaining bytes in the configuration descriptor.
			 *  \param[in,out] CurrConfigLoc      Pointer to the current position in the configuration descriptor.
			 *  \param[in]     ComparatorRoutine  Name of the comparator search function to use on the configuration descriptor.
			 *
			 *  \return Value of one of the members of the \ref DSearch_Comp_Return_ErrorCodes_t enum.
			 *
			 *  Usage Example:
			 *  \code
			 *  uint8_t EndpointSearcher(void* CurrentDescriptor); // Comparator Prototype
			 *
			 *  uint8_t EndpointSearcher(void* CurrentDescriptor)
			 *  {
			 *     if (DESCRIPTOR_TYPE(CurrentDescriptor) == DTYPE_Endpoint)
			 *         return DESCRIPTOR_SEARCH_Found;
			 *     else
			 *         return DESCRIPTOR_SEARCH_NotFound;
			 *  }
			 *
			 *  //...
			 *
			 *  // After retrieving configuration descriptor:
			 *  if (USB_Host_GetNextDescriptorComp(&BytesRemaining, &CurrentConfigLoc, EndpointSearcher) ==
			 *      Descriptor_Search_Comp_Found)
			 *  {
			 *      // Do something with the endpoint descriptor
			 *  }
			 *  \endcode
			 */
			int usb_get_next_descriptor_comp(uint16_t* const BytesRem,
			                                  const void** CurrConfigLoc,
			                                  config_comparator_ptr_t const ComparatorRoutine); 

		/* Inline Functions: */
			/** Skips over the current sub-descriptor inside the configuration descriptor, so that the pointer then
			    points to the next sub-descriptor. The bytes remaining value is automatically decremented.
			 *
			 * \param[in,out] BytesRem       Pointer to the number of bytes remaining of the configuration descriptor.
			 * \param[in,out] CurrConfigLoc  Pointer to the current descriptor inside the configuration descriptor.
			 */
			static inline void usb_get_next_descriptor(uint16_t* const BytesRem,
			                                         const void** CurrConfigLoc);

			static inline void usb_get_next_descriptor(uint16_t* const BytesRem,
			                                         const void** CurrConfigLoc)
			{
				uint16_t CurrDescriptorSize = DESCRIPTOR_CAST(*CurrConfigLoc, usb_descriptor_header_t).Size;
				if (*BytesRem < CurrDescriptorSize)
				  CurrDescriptorSize = *BytesRem;

				*CurrConfigLoc  = (const void*)((uintptr_t)*CurrConfigLoc + CurrDescriptorSize);
				*BytesRem      -= CurrDescriptorSize;
			}

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif


/** @} */

