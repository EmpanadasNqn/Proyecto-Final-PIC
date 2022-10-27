




//---------------master receiver firmaware----------------
void main()
{
  /*TRISD = 0x00;
  PORTD = 0x00;*/
  set_master();
  while(1)
  {
    start(); // I2C Start Sequence
    write(0x41); // I2C Slave Device Address 0x40 + Read
    PORTD = read(); // Read Data From Slave
    stop(); // I2C Stop Sequence
    __delay_ms(100);
  }
}
