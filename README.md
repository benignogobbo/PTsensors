# PTsensors
ADT7420/MS56xx sensors readout via USB through FT232H or via GPIO (Raspberry Pi)

Dependencies

Using USB: libftdi, libusb, https://github.com/devttys0/libmpsse

Using GPIO: libi2c

USB readout consists of an USB to I2C FT232H, followed by a TCA9548A 8-channel I2C switch connected to up to eight couples 
of ADT7420 and MS5607 or MS5611 sensors:

   USB --->  FT232H ---> TCA9548A ---(1-8)---> ADT7420 + MS5607|MS5611
 
 GPIO readout removes FT232H and connects an I2C bus line directly to ADT7420
