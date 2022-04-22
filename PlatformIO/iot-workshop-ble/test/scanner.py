from bluezero import adapter, async_tools, device

mainloop = async_tools.EventLoop()
dongles = adapter.list_adapters()
dongle = adapter.Adapter(dongles[0])


def on_device_found(bz_device_obj: device.Device):
    try:
        if(bz_device_obj.name == "iot-workshop-temp"):
            print(bz_device_obj.address)
            print(bz_device_obj.name)
            data = bz_device_obj.manufacturer_data[list(
                bz_device_obj.manufacturer_data.keys())[0]]
            data = ''.join([chr(x) for x in data])
            temp, fault = data.split(',')
            print(temp)
            print(fault)
    except:
        pass


dongle.on_device_found = on_device_found
dongle.show_duplicates()
dongle.start_discovery()

try:
    mainloop.run()
except KeyboardInterrupt:
    dongle.stop_discovery()
    mainloop.quit()
