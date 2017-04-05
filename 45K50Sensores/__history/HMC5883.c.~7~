// HMC5883 required Registers

#define W_DATA   0x3C     //'Used to perform a Write operation
#define R_DATA   0x3D     //'Used to perform a Read operation
#define CON_A    0x00
#define CON_B    0x01     //'Send continuous MeVARurement mode.
#define MOD_R    0x02     //'Read/Write Register, Selects the operating mode. Default = Single meVARurement
#define X_MSB    0x03     //'Read Register, Output of X MSB 8-bit value.
#define X_LSB    0x04     //'Read Register, Output of X LSB 8-bit value.
#define Z_MSB    0x05     //'Read Register, Output of Z MSB 8-bit value.
#define Z_LSB    0x06     //'Read Register, Output of Z LSB 8-bit value.
#define Y_MSB    0x07     //'Read Register, Output of Y MSB 8-bit value.
#define Y_LSB    0x08     //'Read Register, Output of Y LSB 8-bit value.

#use I2C(MASTER, SLOW, SDA = PIN_B0, SCL = PIN_B1) 

int8 M_data[6]; //6 units 8 bit Measured datas
int16 x = 0, y = 0, z = 0;
int16 data[3];

void hmc5883_write(int add, int data){
    i2c_start();
    i2c_write(W_DATA);
    i2c_write(add);
    i2c_write(data);
    i2c_stop();
}
      
int16 hmc5883_read(int add){
    int retval;
    i2c_start();
    i2c_write(W_DATA);
    i2c_write(add);
    i2c_start();
    i2c_write(R_DATA);
    retval=i2c_read(0);
    i2c_stop();
    return retval;
}
 
void hmc5883_init(){
    hmc5883_write(MOD_R,  0x00);
    delay_ms(100);
    hmc5883_write(CON_A,  0x10);
    delay_ms(100);
    hmc5883_write(CON_B,  0x20);
    delay_ms(100);
}

void hmc5883_getXYZ(){   
   M_data[0]=hmc5883_read(0x04); //Read X (LSB)
   M_data[1]=hmc5883_read(0x03); //Read X (MSB)
   M_data[2]=hmc5883_read(0x08); //Read Y (LSB)
   M_data[3]=hmc5883_read(0x07); //Read Y (MSB)
   M_data[4]=hmc5883_read(0x06); //Read Z (LSB)
   M_data[5]=hmc5883_read(0x05); //Read Z (MSB)

   x = make16(M_data[1],M_data[0]);
   y = make16(M_data[3],M_data[2]);
   z = make16(M_data[5],M_data[4]);
}
