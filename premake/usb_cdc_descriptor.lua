local class_interface_config = {}
local class_endpoint_config = {}

local base = require 'usb_descriptor'

local _M = {
	class_interface_config = class_interface_config,
	class_endpoint_config = class_endpoint_config,
}

class_interface_config.CDC_ABSTRACT_CONTROL = {}
function class_interface_config.CDC_ABSTRACT_CONTROL.struct( f, offset, desc )
	f.x(offset,'usb_cdc_functional_header_t header;')
	f.x(offset,'usb_cdc_functional_call_t call;')
	f.x(offset,'usb_cdc_functional_abstract_control_t abstract_control;')
	f.x(offset,'usb_cdc_functional_union_t<'..#desc.slaves..'> union_;')
end
function class_interface_config.CDC_ABSTRACT_CONTROL.build( f,offset, desc )
	f.x(offset,'header : {')
	base.append_descr_header(f,offset+1,{length='sizeof(usb_cdc_functional_header_t)',
		type='CDC_INTERFACE_DESCRIPTOR_TYPE'})
	f.x(offset+1,'bDescriptorSubtype: CDC_FUNCTIONAL_HEADER,')
	f.x(offset+1,'bcdCDC : USB_BCB(0x01,0x10),')
	f.x(offset,'},')
	
	f.x(offset,'call : {')
	base.append_descr_header(f,offset+1,{length='sizeof(usb_cdc_functional_call_t)',
		type='CDC_INTERFACE_DESCRIPTOR_TYPE'})
	f.x(offset+1,'bDescriptorSubtype: CDC_FUNCTIONAL_CALL,')
	f.x(offset+1,'bmCapabilities : 0,')
	f.x(offset+1,'bDataInterface : '..desc.data_interface..',')
	f.x(offset,'},')

	f.x(offset,'abstract_control : {')
	base.append_descr_header(f,offset+1,{length='sizeof(usb_cdc_functional_abstract_control_t)',
		type='CDC_INTERFACE_DESCRIPTOR_TYPE'})
	f.x(offset+1,'bDescriptorSubtype: CDC_FUNCTIONAL_ABSTRACT_CONTROL,')
	f.x(offset+1,'bmCapabilities : 0x02,')
	f.x(offset,'},')

	f.x(offset,'union_ : {')
	base.append_descr_header(f,offset+1,{length='sizeof(usb_cdc_functional_union_t<'..#desc.slaves..'>)',
		type='CDC_INTERFACE_DESCRIPTOR_TYPE'})
	f.x(offset+1,'bDescriptorSubtype: CDC_FUNCTIONAL_UNION,')
	f.x(offset+1,'bMasterInterface : '..desc.master..',')
	f.x(offset+1,'bSlaveInterface : {'..table.concat(desc.slaves,', ') .. '},' )
	f.x(offset,'},')
end

return _M
