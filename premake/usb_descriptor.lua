local _M = {}

local function writer( file )
	local res = {
		f = file
	}
	function res.header(classes)
		file:write('#include <cstdint>\n')
		file:write('#include "usb/core/usb_defs.h"\n')
		file:write('#include "usb/core/usb_std.h"\n')
		for _,v in ipairs(classes) do
			file:write('#include "usb/' .. v .. '/usb_' .. v .. '_defs.h"\n')
		end
	end
	function res.l( l )
		file:write( l .. '\n')
	end
	function res.x( o, l )
		file:write( string.rep('\t',o) .. l .. '\n')
	end
	function res.footer()
		file:write[[
const void* USB_Std::get_device_descriptor(uint32_t& length) {
	length = sizeof(device_descriptor);
	return &device_descriptor;
}

const void* USB_Std::get_config_descriptor(uint32_t& length) {
	length = sizeof(config_descriptor1);
	return &config_descriptor1;
}

const void* USB_Std::get_string(uint8_t idx,uint32_t& length) {
	if (idx >= (sizeof(string_descripors)/sizeof(string_descripors[0]))) 
		return 0;
	length = string_descripors[idx]->bLength;
	return string_descripors[idx];
}

]]
	end
	return res
end


local function write_usb_string( f, name, str )
	local len = #str
	f.l('USB_DECL_STRING(' .. name..',u"' .. str .. '");')
end

local function add_string( name, str )
	if str then
		for i,v in ipairs(_M.strings) do
			if v.str == str then
				return i
			end
		end 
		local i = #_M.strings + 1
		local sname = name .. tostring(i) .. '_str'
		table.insert(_M.strings,{name=sname,str=str})
		return i
	else
		return 0
	end
end
_M.add_string = add_string

function append_descr_header( f, offset, desc )
	f.x(offset,'header : {')
	f.x(offset+1,'bLength : ' .. desc.length .. ',')
	f.x(offset+1,'bDescriptorType : ' .. desc.type .. ',')
	f.x(offset,'},')
end

_M.append_descr_header = append_descr_header

local function append_descr_struct( f,offset, name, desc )
	if name then
		f.x(offset,''..name..' : {')
	else
		f.x(offset,'{')
	end
	append_descr_header(f,offset+1,desc)
	for _,v in ipairs(desc) do
		f.x(offset+1,v)
	end
	f.x(offset,'},')
end

_M.append_descr_struct = append_descr_struct


local function build_device_descr(f,desc)


	f.l('#define USBD_LANGID_STRING ' .. tostring(1033))
	
	f.l('static const usb_device_descr_t device_descriptor __attribute__ ((aligned (4))) = {')
	append_descr_header(f,1,{length='usb_device_descr_size',type='USB_DESC_TYPE_DEVICE'})

	f.x(1,'bcdUSB : USB_BCB(2,0),')
	f.x(1,'bDeviceClass : 0x00,')
	f.x(1,'bDeviceSubClass : 0x00,')
	f.x(1,'bDeviceProtocol : 0x00,')
	f.x(1,'bMaxPacketSize0 : USB_MAX_EP0_SIZE,')
	f.x(1,'idVendor : USBD_VID,')
	f.x(1,'idProduct : USBD_PID,')
	f.x(1,'bcdDevice : USB_BCB(2,0),')
	f.x(1,'iManufacturer : '..add_string('manufacturer',desc.manufacturer)..',')
	f.x(1,'iProduct : '..add_string('product',desc.product)..',')
	f.x(1,'iSerialNumber : '..add_string('serialnumber',desc.serialnumber)..',')
	f.x(1,'bNumConfigurations : USBD_NUM_CONFIGURATIONS')
	f.l('};')

	f.l('static const usb_lang_id_descr_t<1> lang_id __attribute__ ((aligned (4))) = {')
	f.x(1,'header : {')
	f.x(2,'bLength : sizeof(usb_lang_id_descr_t<1>),')
	f.x(2,'bDescriptorType : USB_DESC_TYPE_STRING')
	f.x(1,'},')
	f.x(1,'wLANGID :{ USBD_LANGID_STRING }')
	f.l('};')
	
end



local function config_interface( f, offset, name, desc )
	append_descr_struct(f,offset,name,{
			length = 'usb_interface_descr_size',
			type = 'USB_DESC_TYPE_INTERFACE',
			'bInterfaceNumber : ' .. desc.interface_num .. ',',
			'bAlternateSetting : ' .. (desc.alt_num or 0) .. ',',
			'bNumEndpoints : ' .. (desc.numendpoints or 0) .. ',',
			'bInterfaceClass : ' .. 'USB_INTERFACE_CLASS_' .. desc.class .. ',',
			'bInterfaceSubClass : ' .. desc.class .. '_SUBCLASS_' .. desc.subclass .. ',',
			'bInterfaceProtocol : ' .. desc.class .. '_PROTOCOL_' .. desc.protocol .. ',',
			'iInterface : ' .. add_string('interface',desc.name)..',',
	})
end


local class_interface_config = {}
local class_endpoint_config = {}

local function ep_name(e) 
	return 'ep' .. e.idx .. (e.out and 'out' or 'in')
end

local function build_config_struct(f,desc)
	f.l('struct config_descriptor'..desc.config_num..'_t {')
	f.x(1,'usb_config_descr_t header;')
	for i,interface in ipairs(desc.interfaces) do
		interface.config = desc
		interface.interface_num = i - 1
		f.x(1,'struct ' .. interface.name .. '_t {' )
		f.x(2,'usb_interface_descr_t header;')
		if interface.alternate and #interface.alternate > 0 then
			f.x(2,'usb_interface_descr_t alternate['..#interface.alternate..'];')
		end
		local iff = class_interface_config[interface.class .. '_' ..interface.subclass]
		if iff then
			if iff.struct then
				f.x(2,'struct cs_t {')
				iff.struct(f,3,interface)
				f.x(2,'} __attribute__((packed)) cs;')
			elseif iff.struct_name then
				f.x(2,'struct ' .. iff.struct_name .. ' cs;')
			end
		end
		if interface.endpoints then
			for _,e in ipairs(interface.endpoints) do
				f.x(2,'struct ' .. ep_name(e) .. '_t {')
				
				local cls_ep = class_endpoint_config[interface.class .. '_' .. interface.subclass]
				if cls_ep and cls_ep.standard then
					cls_ep.standard.struct(f,3,e)
				else
					f.x(3,'usb_endpoint_descr_t header;')
				end
				if cls_ep then
					cls_ep.struct(f,3,e)
				end
				f.x(2,'} ' .. ep_name(e) .. ';')
			end
		end
		f.x(1,'} __attribute__((packed)) '..interface.name..';')
	end
	f.l('}__attribute__((packed));')
end

local function build_config_descr(f,desc)
	
	f.l('static const config_descriptor'..desc.config_num..'_t config_descriptor'..desc.config_num..' __attribute__ ((aligned (4))) = {')
	-- for _,l in ipairs(total_conf) do
	-- 	f.l('\t' .. l)
	-- end
	append_descr_struct(f,1,'header',{
		length = 'usb_config_descr_size',
		type = 'USB_DESC_TYPE_CONFIGURATION',
		'wTotalLength : sizeof(config_descriptor' .. desc.config_num .. '_t),',
		'bNumInterfaces : ' .. #desc.interfaces .. ',',
		'bConfigurationValue : '..desc.config_num..',',
		'iConfiguration : ' .. add_string('configuration',desc.name) .. ',',
		'bmAttributes : 0x80 | 0, // bus powered (D6 zero)',
		'bMaxPower : 100 / 2, // 100ma',
	})
	for i,interface in ipairs(desc.interfaces) do
		f.x(1,'' .. interface.name .. ' : {')
		config_interface(f,2,'header',interface)
		if interface.alternate and #interface.alternate > 0 then
			f.x(2,'alternate : {')
			for a,alt in ipairs(interface.alternate) do
				alt.alt_num = a
				config_interface(f,3,nil,setmetatable(interface,{__index=alt}))
			end
			f.x(2,'},')
		end
		local iff = class_interface_config[interface.class .. '_' ..interface.subclass]
		if iff then
			f.x(2,'cs : {')
			iff.build(f,3,interface)
			f.x(2,'},')
		end
		if interface.endpoints then
			for _,e in ipairs(interface.endpoints) do
				f.x(2,'' .. ep_name(e) .. ' : {')
				local cls_ep = class_endpoint_config[interface.class .. '_' .. interface.subclass]
				if cls_ep and cls_ep.standard then
					cls_ep.standard.build(f,3,e)
				else
					append_descr_struct(f,3,'header',{
						length = 'usb_endpoint_descr_size',
						type = 'USB_DESC_TYPE_ENDPOINT',
						'bEndpointAddress : '..e.idx..(e.out and '|0x80' or '')..',',
						'bmAttributes : ' .. (e.attr or '0') .. ',',
						'wMaxPacketSize : '..e.size..',',
						'bInterval : ' .. (e.interval or 0) .. ',',
					})
				end
				if cls_ep then
					cls_ep.build(f,3,e)
				end
				f.x(2,'},')
			end
		end
		f.x(1,'},')
	end
	f.l('};')
end

function _M.descriptor( desc )
	_M.strings = {}

	for _,v in ipairs(desc.classes) do
		local cls_m = require ('usb_' .. v .. '_descriptor')
		for k,v in pairs(cls_m.class_interface_config) do
			class_interface_config[k]=v
		end
		for k,v in pairs(cls_m.class_endpoint_config) do
			class_endpoint_config[k]=v
		end
	end
	
	os.mkdir(path.join(_WORKING_DIR,'build','src'))
	local f = path.join(_WORKING_DIR,'build','src','usb_descriptors.cpp')
	local file = io.open(f,'w')
	local w = writer(file)
	w.header(desc.classes)
	w.l('#define USBD_VID ' .. tostring(desc.VID))
	w.l('#define USBD_PID ' .. tostring(desc.PID))
	w.l('#define USBD_NUM_CONFIGURATIONS ' .. tostring(#desc.configs))
	build_device_descr(w,desc)
	for i,conf in ipairs(desc.configs) do
		conf.config_num = i
		build_config_struct(w,conf)
		build_config_descr(w,conf)
	end
	for _,v in ipairs(_M.strings) do
		write_usb_string(w,v.name,v.str)
	end
	w.l('static const usb_descr_header_t* string_descripors[] = {')
	w.x(1,'&lang_id.header,')
	for _,v in ipairs(_M.strings) do
		w.x(1,'&'..v.name..'.header,')
	end
	w.l('};')
	w.footer()
	file:close()
	return f
end


return _M