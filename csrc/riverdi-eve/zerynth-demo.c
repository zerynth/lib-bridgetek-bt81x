#include "platform.h"
#include "App_Common.h"

/* Global used for buffer optimization */
Gpu_Hal_Context_t host, *phost;

static void SAMAPP_GPU_Ball_Stencil();
#if 1

void SAMAPP_Animation1()
{
#define ANIM_ADDR1 4270272 //address of "bicycle.anim.object" from Flash map after generating Flash
	/* Switch Flash to FULL Mode */
  printf("to full\n");
	Gpu_CoCmd_FlashHelper_SwitchFullMode(phost);
  printf("full\n");

	uint16_t frame = 0;
	while (1)
	{
    printf("a\n");
		Gpu_CoCmd_Dlstart(phost);
		App_WrCoCmd_Buffer(phost, CLEAR_COLOR_RGB(0, 0, 0));
		App_WrCoCmd_Buffer(phost, CLEAR(1, 1, 1));
    printf("b\n");
		Gpu_CoCmd_AnimFrame(phost, DispWidth / 2, DispHeight / 2, ANIM_ADDR1, frame);
		App_WrCoCmd_Buffer(phost, DISPLAY());
    printf("c\n");
		Gpu_CoCmd_Swap(phost);
		App_Flush_Co_Buffer(phost);
    printf("d\n");
		Gpu_Hal_WaitCmdfifo_empty(phost);
    printf("e\n");
		frame++;
		if (frame == 1635)
			frame=0;
    printf("sleep\n");
    Gpu_Hal_Sleep(100);
	}
}

void SAMAPP_Animation2()
{
#define ANIM_ADDR2 408576 //address of "bicycle.anim.object" from Flash map after generating Flash
	/* Switch Flash to FULL Mode */
	Gpu_CoCmd_FlashHelper_SwitchFullMode(phost);

	uint16_t frame = 0;
	while (1)
	{
		Gpu_CoCmd_Dlstart(phost);
		App_WrCoCmd_Buffer(phost, CLEAR_COLOR_RGB(0, 0, 0));
		App_WrCoCmd_Buffer(phost, CLEAR(1, 1, 1));
		Gpu_CoCmd_AnimFrame(phost, DispWidth / 2, DispHeight / 2, ANIM_ADDR2, frame);
		App_WrCoCmd_Buffer(phost, DISPLAY());
		Gpu_CoCmd_Swap(phost);
		App_Flush_Co_Buffer(phost);
		Gpu_Hal_WaitCmdfifo_empty(phost);
		frame++;
		if (frame == 25)
			frame =0;
	}
}
#endif


void ftdi_logo() {

  Gpu_CoCmd_Dlstart(phost);

  Gpu_Hal_WrCmd32(phost, CLEAR(1, 1, 1)); // clear screen

  // Gpu_Hal_WrCmd32(phost, BEGIN(BITMAPS)); // start drawing bitmaps
  // Gpu_Hal_WrCmd32(phost, VERTEX2II(220, DispHeight/3, 31, 'F')); // ascii F in font 31
  // Gpu_Hal_WrCmd32(phost, VERTEX2II(244, DispHeight/3, 31, 'T')); // ascii T
  // Gpu_Hal_WrCmd32(phost, VERTEX2II(270, DispHeight/3, 31, 'D')); // ascii D
  // Gpu_Hal_WrCmd32(phost, VERTEX2II(299, DispHeight/3, 31, 'I')); // ascii I
  // Gpu_Hal_WrCmd32(phost, END());

  // Gpu_CoCmd_Text(phost, (DispWidth/2), DispHeight/2, 31, OPT_CENTERX,"FTDI");

  Gpu_CoCmd_Track(phost, 192, DispHeight/3 + 22, 40, 40, 1);

  // Gpu_Hal_WrCmd32(phost, COLOR_RGB(160, 22, 22)); // change color to red
  Gpu_Hal_WrCmd32(phost, COLOR_RGB(0xff, 0xff, 0xff)); // change color to red
  Gpu_Hal_WrCmd32(phost, TAG(1));

  // Gpu_Hal_WrCmd32(phost, BEGIN(POINTS)); // start drawing points
  // Gpu_Hal_WrCmd32(phost, POINT_SIZE(800)); // set point size to 20 pixels in radius
  // Gpu_Hal_WrCmd32(phost, VERTEX2II(192, DispHeight/3 + 22, 0, 0)); // red point
  // Gpu_Hal_WrCmd32(phost, END());

  Gpu_CoCmd_Button(phost, DispHeight/3, DispWidth/3, DispWidth/5, DispHeight/5, 31, 0, "WiFi!!!");
  Gpu_Hal_WrCmd32(phost, DISPLAY()); // display the image

  Gpu_CoCmd_Swap(phost);
  // App_Flush_Co_Buffer(phost);
  Gpu_Hal_WaitCmdfifo_empty(phost);

  while (1) {
    uint32_t tt = Gpu_Hal_Rd32(phost, REG_TRACKER);
    uint8_t tag = tt & 0xff;
    if (tag == 1) {
      printf("PRESSED!\n");
    }
    vosThSleep(TIME_U(100, MILLIS));
  }
}

C_NATIVE(_app_main) {
  NATIVE_UNWARN();

  phost = &host;

  printf("> app common init\n");
  /* Init HW Hal */
  App_Common_Init(&host);

  /* Screen Calibration*/
  // App_Calibrate_Screen(&host);

  // printf("> app show logo\n");
  /* Show Bridgetek logo */
  // App_Show_Logo(&host);

  // ftdi_logo();

  // while (1) {
  //   vosThSleep(TIME_U(1000, MILLIS));
  // }
  // printf("> play sound\n");
  // App_Play_Sound(&host, 0x41, 0xFF, 0x6C);

  // Gpu_Hal_Sleep(1000);
  // printf("> nice sound\n");

  // printf("> animation2\n");
  // SAMAPP_Animation1();
  // SAMAPP_Animation2();

  printf("> stencil\n");
  /* Main application - endless loop */
  // SAMAPP_GPU_Ball_Stencil();

  printf("> kill\n");
  /* Close all the opened handles */

  // Gpu_Hal_Close(phost);
  // Gpu_Hal_DeInit();

  return ERR_OK;
}

static float
lerp (float t, float a, float b)
{
  return (float)((1 - t) * a + t * b);
}

static float
smoothlerp (float t, float a, float b)
{
  float lt = 3 * t * t - 2 * t * t * t;
  return lerp(lt, a, b);
}

static void
SAMAPP_GPU_Ball_Stencil()
{
  int16_t xball = (DispWidth/2),yball = 120,rball = (DispWidth/8);
  int16_t numpoints = 6,numlines = 8,i,asize,aradius,gridsize = 20;
  int32_t asmooth,dispr = (DispWidth - 10),displ = 10,dispa = 10,dispb = (DispHeight - 10),xflag = 1,yflag = 1;

  dispr -= ((dispr - displ)%gridsize);
  dispb -= ((dispb - dispa)%gridsize);

  /* endless loop */
  while(1)
    {
      if(((xball + rball + 2) >= dispr) || ((xball - rball - 2) <= displ))
        {
          xflag ^= 1;
          Gpu_Hal_Wr8(phost, REG_PLAY,1);
        }

      if(((yball + rball + 8) >= dispb) || ((yball - rball - 8) <= dispa))
        {
          yflag ^= 1;
          Gpu_Hal_Wr8(phost, REG_PLAY,1);
        }

      if(xflag)
        xball += 2;
      else
        xball -= 2;

      if(yflag)
        yball += 8 ;
      else
        yball -= 8;

      App_WrDl_Buffer(phost, CLEAR_COLOR_RGB(128, 128, 0) );
      App_WrDl_Buffer(phost, CLEAR(1, 1, 1));
      App_WrDl_Buffer(phost, STENCIL_OP(INCR,INCR) );
      App_WrDl_Buffer(phost, COLOR_RGB(0, 0, 0) );

      /* draw grid */
      App_WrDl_Buffer(phost, LINE_WIDTH(16));
      App_WrDl_Buffer(phost, BEGIN(LINES));

      for(i=0;i<=((dispr - displ)/gridsize);i++)
        {
          App_WrDl_Buffer(phost, VERTEX2F((displ + i*gridsize)*16,dispa*16));
          App_WrDl_Buffer(phost, VERTEX2F((displ + i*gridsize)*16,dispb*16));
        }

      for(i=0;i<=((dispb - dispa)/gridsize);i++)
        {
          App_WrDl_Buffer(phost, VERTEX2F(displ*16,(dispa + i*gridsize)*16));
          App_WrDl_Buffer(phost, VERTEX2F(dispr*16,(dispa + i*gridsize)*16));
        }

      App_WrDl_Buffer(phost, END());
      App_WrDl_Buffer(phost, COLOR_MASK(0,0,0,0) );
      App_WrDl_Buffer(phost, POINT_SIZE(rball*16) );
      App_WrDl_Buffer(phost, BEGIN(FTPOINTS));
      App_WrDl_Buffer(phost, VERTEX2F(xball*16,yball*16));
      App_WrDl_Buffer(phost, STENCIL_OP(INCR,ZERO) );
      App_WrDl_Buffer(phost, STENCIL_FUNC(GEQUAL,1,255));

      for(i=1;i<=numpoints;i++)
        {
          asize = i*rball*2/(numpoints + 1);
          asmooth = (int16_t)smoothlerp((float)((float)(asize)/(2*(float)rball)),0,2*(float)rball);

          if(asmooth > rball)
            {
              int32_t tempsmooth;

              tempsmooth = asmooth - rball;
              aradius = (rball*rball + tempsmooth*tempsmooth)/(2*tempsmooth);
              App_WrDl_Buffer(phost, POINT_SIZE(aradius*16) );
              App_WrDl_Buffer(phost, VERTEX2F((xball - aradius + tempsmooth)*16,yball*16));
            }
          else
            {
              int32_t tempsmooth;

              tempsmooth = rball - asmooth;
              aradius = (rball*rball + tempsmooth*tempsmooth)/(2*tempsmooth);
              App_WrDl_Buffer(phost, POINT_SIZE(aradius*16) );
              App_WrDl_Buffer(phost, VERTEX2F((xball+ aradius - tempsmooth)*16,yball*16));
            }
          }

        App_WrDl_Buffer(phost, END());
        App_WrDl_Buffer(phost, BEGIN(LINES));

        /* draw lines - line should be at least radius diameter */
        for(i=1;i<=numlines;i++)
          {
            asize = (i*rball*2/numlines);
            asmooth = (int16_t)smoothlerp((float)((float)(asize)/(2*(float)rball)),0,2*(float)rball);
            App_WrDl_Buffer(phost, LINE_WIDTH(asmooth * 16));
            App_WrDl_Buffer(phost, VERTEX2F((xball - rball)*16,(yball - rball )*16));
            App_WrDl_Buffer(phost, VERTEX2F((xball + rball)*16,(yball - rball )*16));
          }
        App_WrDl_Buffer(phost, END());

        App_WrDl_Buffer(phost, COLOR_MASK(1,1,1,1) );//enable all the colors
        App_WrDl_Buffer(phost, STENCIL_FUNC(ALWAYS,1,255));
        App_WrDl_Buffer(phost, STENCIL_OP(KEEP,KEEP));
        App_WrDl_Buffer(phost, COLOR_RGB(255, 255, 255) );
        App_WrDl_Buffer(phost, POINT_SIZE(rball*16) );
        App_WrDl_Buffer(phost, BEGIN(FTPOINTS));
        App_WrDl_Buffer(phost, VERTEX2F((xball - 1)*16,(yball - 1)*16));
        App_WrDl_Buffer(phost, COLOR_RGB(0, 0, 0) );//shadow
        App_WrDl_Buffer(phost, COLOR_A(160) );
        App_WrDl_Buffer(phost, VERTEX2F((xball+16)*16,(yball+8)*16));
        App_WrDl_Buffer(phost, COLOR_A(255) );
        App_WrDl_Buffer(phost, COLOR_RGB(255, 255, 255) );
        App_WrDl_Buffer(phost, VERTEX2F(xball*16,yball*16));
        App_WrDl_Buffer(phost, COLOR_RGB(255, 0, 0) );
        App_WrDl_Buffer(phost, STENCIL_FUNC(GEQUAL,1,1));
        App_WrDl_Buffer(phost, STENCIL_OP(KEEP,KEEP));
        App_WrDl_Buffer(phost, VERTEX2F(xball*16,yball*16));

        App_WrDl_Buffer(phost, END());
        App_WrDl_Buffer(phost,DISPLAY());

        /* Download the DL into DL RAM */
        App_Flush_DL_Buffer(phost);

        /* Do a swap */
        Gpu_Hal_DLSwap(phost,DLSWAP_FRAME);
      } /* while */
}

