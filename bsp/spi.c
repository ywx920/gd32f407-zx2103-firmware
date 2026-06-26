#include "gd32f4xx.h"
#include "spi.h"

//管脚定义
const SPI_PIX_TAB_TYP spiTab[SPI_NUM] = {
    //port cs sck miso mosi
    {0, 0, 0, 0, 0}, //第0路SPI管脚定义  未验证
    {GPIOB, GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15}, //第1路SPI管脚定义
};
const uint32_t spiCtler[SPI_NUM] = \
    {SPI0,SPI1};
    
//复用ID
const static uint32_t spiAfTab[SPI_NUM] = {
    GPIO_AF_5, GPIO_AF_5
};

const static rcu_periph_enum spiRcuTab[SPI_NUM] = \
    {RCU_SPI0,RCU_SPI1};

const static rcu_periph_enum spiGpioRcuTab[SPI_NUM] = \
    { (rcu_periph_enum)0,RCU_GPIOB};   //显示类型转换
    
    
/*!
    \brief      SPI GPIO初始化
    \param      uint8_t spix:SPI_CTLER0/SPI_CTLER1
    \retval     void
*/
static void spiGpioInit(uint8_t spix)
{   
    if(spix < SPI_NUM)
    {
        /* SPI GPIO config */
        gpio_af_set(spiTab[spix].port, spiAfTab[spix], \
        spiTab[spix].pinSck | spiTab[spix].pinMiso | spiTab[spix].pinMosi);
        gpio_mode_set(spiTab[spix].port, GPIO_MODE_AF, GPIO_PUPD_NONE, \
        spiTab[spix].pinSck | spiTab[spix].pinMiso | spiTab[spix].pinMosi);
        gpio_output_options_set(spiTab[spix].port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, \
        spiTab[spix].pinSck | spiTab[spix].pinMiso | spiTab[spix].pinMosi);
        
        /* set SPI_NSS as GPIO*/
        gpio_mode_set(spiTab[spix].port,GPIO_MODE_OUTPUT,GPIO_PUPD_NONE,spiTab[spix].pinCs);
        gpio_output_options_set(spiTab[spix].port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, spiTab[spix].pinCs);
        //拉高 置为无效状态
        SET_NSS_HIGH(spix);
        
    }
}

/*!
    \brief      SPI GPIO初始化
    \param      uint8_t spix:SPI_CTLER0/SPI_CTLER1
    \retval     void
*/
void spiInit(uint8_t spix)
{   
    spi_parameter_struct spi_init_struct;

    if(spix < SPI_NUM)
    {               
        rcu_periph_clock_enable(spiRcuTab[spix]);
        rcu_periph_clock_enable(spiGpioRcuTab[spix]);
        
        spiGpioInit(spix);
        

        /* SPI1 parameter config */
        spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
        spi_init_struct.device_mode          = SPI_MASTER;
        spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
        spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;//SPI_CK_PL_HIGH_PH_1EDGE / SPI_CK_PL_HIGH_PH_2EDGE
        spi_init_struct.nss                  = SPI_NSS_SOFT; //SPI_NSS_HARD
        spi_init_struct.prescale             = SPI_PSC_32;
        spi_init_struct.endian               = SPI_ENDIAN_MSB;
        spi_init(spiCtler[spix], &spi_init_struct);
        
        /* SPI enable */
    spi_enable(spiCtler[spix]);
    }
}

/*!
    \brief      SPI 数据发送
    \param      uint8_t spix:SPI_CTLER0/SPI_CTLER1
                uint8_t *buf:发送的数据
                uint8_t size:发送的数据大小
    \retval     返回:成功发送数据的长度
*/
uint8_t spiWrite(uint8_t spix, uint8_t *buf, uint8_t size)
{
    SET_NSS_LOW(spix);
    
    SET_NSS_HIGH(spix);
    
    return size;
}

/*!
    \brief      SPI 数据读取
    \param      uint8_t spix:SPI_CTLER0/SPI_CTLER1
                uint8_t *buf:读取的数据存放指针
                uint8_t size:读取的数据大小
    \retval     返回:成功读取数据的长度
*/
uint8_t spiRead(uint8_t spix, uint8_t *buf, uint8_t size)
{
    SET_NSS_LOW(spix);
    SET_NSS_HIGH(spix);
    return size;
}

