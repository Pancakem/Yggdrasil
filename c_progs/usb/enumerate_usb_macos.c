#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFUUID.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>

#include <IOKit/IOCFPlugin.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  int pID;
  int vID;
} device_t;

void str_trim(char *str) {
  int begin = 0;
  int end = strlen(str);
  while (isspace((unsigned char)str[begin]))
    begin++;

  while ((end >= begin) && isspace((unsigned char)str[end]))
    end--;

  memmove(str, str + begin, end - begin + 1);
  str[end] = '\0';
}

bool get_string_from_descriptor_idx(IOUSBDeviceInterface **dev, UInt8 idx,
                                    char **str) {
  IOUSBDevRequest request;
  IOReturn ioret;
  char buffer[4086] = {0};
  CFStringRef cfstr;
  CFIndex len;

  if (str != NULL)
    *str = NULL;

  request.bmRequestType =
      USBmakebmRequestType(kUSBIn, kUSBStandard, kUSBDevice);
  request.bRequest = kUSBRqGetDescriptor;
  request.wValue = (kUSBStringDesc << 8) | idx;
  request.wIndex = 0x409;
  request.wLength = sizeof(buffer);
  request.pData = buffer;

  ioret = (*dev)->DeviceRequest(dev, &request);
  if (ioret != kIOReturnSuccess)
    return false;

  if (str == NULL || request.wLenDone <= 2)
    return true;

  // parsing the data
  cfstr = CFStringCreateWithBytes(NULL, (const UInt8 *)buffer + 2,
                                  request.wLenDone - 2,
                                  kCFStringEncodingUTF16LE, 0);
  len = CFStringGetMaximumSizeForEncoding(CFStringGetLength(cfstr),
                                          kCFStringEncodingUTF8) +
        1;

  if (len < 0) {
    CFRelease(cfstr);
    return true;
  }

  *str = calloc(1, (size_t)len);
  CFStringGetCString(cfstr, *str, len, kCFStringEncodingUTF8);
  str_trim(*str);

  CFRelease(cfstr);

  return true;
}

void print_dev_info(IOUSBDeviceInterface **dev) {
  char *str;
  UInt8 si;
  UInt16 u16v;

  if ((*dev)->GetDeviceVendor(dev, &u16v) == kIOReturnSuccess)
    printf("\tVendor ID: %u\n", u16v);

  if ((*dev)->GetDeviceProduct(dev, &u16v) == kIOReturnSuccess)
    printf("\tProduct ID: %u\n", u16v);

  if ((*dev)->USBGetManufacturerStringIndex(dev, &si) == kIOReturnSuccess) {
    get_string_from_descriptor_idx(dev, si, &str);
    printf("\tManufacturer: %s\n", str);
    free(str);
  }

  if ((*dev)->USBGetProductStringIndex(dev, &si) == kIOReturnSuccess) {
    get_string_from_descriptor_idx(dev, si, &str);
    printf("\tProduct: %s\n", str);
    free(str);
  }

  if ((*dev)->USBGetSerialNumberStringIndex(dev, &si) == kIOReturnSuccess) {
    get_string_from_descriptor_idx(dev, si, &str);
    printf("\tSerial: %s\n", str);
    free(str);
  }

  if ((*dev)->GetDeviceSpeed(dev, &si) == kIOReturnSuccess) {
    printf("\tSpeed: ");
    switch (si) {
    case kUSBDeviceSpeedLow:
      printf("Low\n");
      break;
    case kUSBDeviceSpeedFull:
      printf("Full\n");
      break;
    case kUSBDeviceSpeedHigh:
      printf("High\n");
      break;
    case kUSBDeviceSpeedSuper:
      printf("Super\n");
      break;
    case kUSBDeviceSpeedSuperPlus:
      printf("Super Plus\n");
      break;
    case kUSBDeviceSpeedSuperPlusBy2:
      printf("Super Plus 2\n");
      break;
    }
  }

  if ((*dev)->GetConfiguration(dev, &si) == kIOReturnSuccess)
    printf("\tCurrent Config: %u\n", si);
}

void enumerate_usb(void) {
  io_registry_entry_t entry = 0;
  io_iterator_t iter = 0;
  io_service_t service = 0;
  kern_return_t kret;

  entry = IORegistryGetRootEntry(kIOMainPortDefault);
  if (entry == 0)
    return;

  kret = IORegistryEntryCreateIterator(entry, kIOUSBPlane,
                                       kIORegistryIterateRecursively, &iter);
  if (kret != KERN_SUCCESS || iter == 0)
    return;

  while ((service = IOIteratorNext(iter))) {
    IOCFPlugInInterface **plug = NULL;
    IOUSBDeviceInterface **dev = NULL;
    io_string_t path;
    SInt32 score = 0;
    IOReturn ioret;

    kret = IOCreatePlugInInterfaceForService(
        service, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plug,
        &score);

    IOObjectRelease(service);
    if (kret != KERN_SUCCESS || plug == NULL)
      continue;

    ioret = (*plug)->QueryInterface(
        plug, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID), (void *)&dev);
    (*plug)->Release(plug);

    if (ioret != kIOReturnSuccess || dev == NULL)
      continue;

    if (IORegistryEntryGetPath(service, kIOServicePlane, path) !=
        KERN_SUCCESS) {
      (*dev)->Release(dev);
      continue;
    }
    printf("Found device \t '%s'\n", path);
    print_dev_info(dev);
    (*dev)->Release(dev);
  }
}

int view_device(SInt32 vendor_id, SInt32 product_id) {
  CFMutableDictionaryRef mD = NULL;
  io_iterator_t it = 0;
  io_service_t usb_ref;
  SInt32 score;
  IOCFPlugInInterface **plugin;
  IOUSBDeviceInterface300 **usb_dev = NULL;
  IOReturn ret;
  IOUSBConfigurationDescriptorPtr config;
  IOUSBFindInterfaceRequest interface_req;
  IOUSBInterfaceInterface300 **usb_interface;
  char out[] = {0x00, 0x00}; // data to send

  char *in;
  UInt32 num_bytes;

  // find device
  mD = IOServiceMatching(kIOUSBDeviceClassName);
  CFDictionaryAddValue(
      mD, CFSTR(kUSBVendorID),
      CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &vendor_id));

  CFDictionaryAddValue(
      mD, CFSTR(kUSBProductID),
      CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &product_id));

  IOServiceGetMatchingServices(kIOMainPortDefault, mD, &it);

  usb_ref = IOIteratorNext(it);
  if (usb_ref == 0) {
    printf("device has not been found");
    return -1;
  }

  IOObjectRelease(it);
  IOCreatePlugInInterfaceForService(usb_ref, kIOUSBDeviceUserClientTypeID,
                                    kIOCFPlugInInterfaceID, &plugin, &score);

  IOObjectRelease(usb_ref);
  (*plugin)->QueryInterface(
      plugin, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID300), (LPVOID)&usb_dev);

  (*plugin)->Release(plugin);

  // open device
  ret = (*usb_dev)->USBDeviceOpen(usb_dev);
  if (ret == kIOReturnSuccess) {
    ret = (*usb_dev)->GetConfigurationDescriptorPtr(usb_dev, 0, &config);
    if (ret != kIOReturnSuccess) {
      printf("could not set active configuration (error: %x)\n", ret);
      return -1;
    }
    (*usb_dev)->SetConfiguration(usb_dev, config->bConfigurationValue);
  } else {
    printf("could not open device (error: %x)\n", ret);
    return -1;
  }

  // find device interface for transfer
  interface_req.bInterfaceClass = kIOUSBFindInterfaceDontCare;
  interface_req.bInterfaceSubClass = kIOUSBFindInterfaceDontCare;
  interface_req.bInterfaceProtocol = kIOUSBFindInterfaceDontCare;
  interface_req.bAlternateSetting = kIOUSBFindInterfaceDontCare;
  (*usb_dev)->CreateInterfaceIterator(usb_dev, &interface_req, &it);

  IOIteratorNext(it); // skip interface
  usb_ref = IOIteratorNext(it);
  IOObjectRelease(it);
  IOCreatePlugInInterfaceForService(usb_ref, kIOUSBInterfaceUserClientTypeID,
                                    kIOCFPlugInInterfaceID, &plugin, &score);

  IOObjectRelease(usb_ref);
  (*plugin)->QueryInterface(plugin,
                            CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID300),
                            (LPVOID)&usb_interface);
  (*plugin)->Release(plugin);

  // open interface
  ret = (*usb_interface)->USBInterfaceOpen(usb_interface);
  if (ret != kIOReturnSuccess) {
    printf("could not open interface (error: %x)\n", ret);
    return -1;
  }

  // send data through pipe 1
  (*usb_interface)->WritePipe(usb_interface, 1, out, sizeof(out));

  // read data through pipe 2
  num_bytes = 64;
  in = malloc(num_bytes);
  ret = (*usb_interface)->ReadPipe(usb_interface, 2, in, &num_bytes);
  if (ret == kIOReturnSuccess)
    printf("read %d bytes\n", num_bytes);
  else
    printf("failed to read (error: %x)\n", ret);

  printf("%s\n", in);
  // clean up
  free(in);
  (*usb_interface)->USBInterfaceClose(usb_interface);
  (*usb_dev)->USBDeviceClose(usb_dev);
  return 0;
}

int main(void) {
  enumerate_usb();
//  view_device(7694, 36910);
}
