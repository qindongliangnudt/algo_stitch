#include <string.h>
#include "common.h"

gdouble g_homoH[4][3][3] = {
 { {3.0, 0.0,     0.0}, {0.0, 4.0,     0.0}, {0, 0, 1} }
,{ {4.0, 0.2,   -100.0}, {-0.2, 4.0, -800.0}, {0, 0, 1} }
,{ {6.0, -1.0, -2000.0}, {-1.0, 6.0, -1600.0}, {0, 0, 1} }
,{ {4.0, 0.0,     0.0}, {0.0, 4.0,     0.0}, {0, 0, 1} }
};

static void imageTransform(gdouble xin, gdouble yin, gdouble *xout, gdouble *yout, gint imgid)
{
    double s = g_homoH[imgid][2][0] * xin + g_homoH[imgid][2][1] * yin + g_homoH[imgid][2][2] * 1.0;
    *xout = (g_homoH[imgid][0][0] * xin + g_homoH[imgid][0][1] * yin + g_homoH[imgid][0][2] * 1.0) / s;
    *yout = (g_homoH[imgid][1][0] * xin + g_homoH[imgid][1][1] * yin + g_homoH[imgid][1][2] * 1.0) / s;
}

static gint algo_stitch_handle(
          ImageInfo *pin_image
        , ImageInfo *pout_image
        , gint nums)
{
    if (nums > 4)
        nums = 4;
    
    gint dstW = pout_image[0].width;
    gint dstH = pout_image[0].height;
    gint dstL = pout_image[0].linesize;
    guchar *dstY = pout_image[0].buf;
    guchar *dstU = dstY + dstL*dstH;
    guchar *dstV = dstU + dstL*dstH/4;

    g_print("dstL:%d. dstW:%d. dstH:%d.\n", dstL, dstW, dstH);
    memset(dstY, 0, dstL*dstH);
    memset(dstU, 128, dstL*dstH/4);
    memset(dstV, 128, dstL*dstH/4);

    gint t;
    gint x, y, xx, yy; //image coordinates
    for (t=0; t<nums; t++)
    {
        gint srcW = pin_image[t].width;
        gint srcH = pin_image[t].height;
        gint srcL = pin_image[t].linesize;
        guchar *srcY = pin_image[t].buf;
        guchar *srcU = srcY + srcL*srcH;
        guchar *srcV = srcU + srcL*srcH/4;
        g_print("src:t%d. srcL:%d. srcW:%d. srcH:%d.\n", t, srcL, srcW, srcH);
	    for (y=0; y<dstH; y++)
	    {
		    for (x=0; x<dstW; x++)
		    {
		        double xin, yin, xout, yout;
		        xin = x;
		        yin = y;
                imageTransform(xin, yin, &xout, &yout, t);
                xx = (int)(xout+0.5);
                yy = (int)(yout+0.5);
                
		        if (yy >= 0 && yy < srcH && xx >= 0 && xx < srcW)
		        {
			        dstY[y*dstL+x] = srcY[yy*srcL+xx];
			        if (x%2==0 && y%2==0) // do u & v
			        {
			            dstU[y/2*dstL/2+x/2] = srcU[yy/2*srcL/2+xx/2];
			            dstV[y/2*dstL/2+x/2] = srcV[yy/2*srcL/2+xx/2];
			        }
		        }
		    }
	    }
    }
    return FILTER_SWAP;
}

Filter algo_stitch_filter = {
    .name = "algo_stitch_filter",
    .func_set = {
        .filter_handle = algo_stitch_handle,
    },
};

gint algo_stitch_init(void)
{
    gint ret = filter_register(IDS_TYPE_STITCH, &algo_stitch_filter);
    if (ret < 0) {
        g_error("algo_stitch_filter register failed with error code:%d\n", ret);
        return FALSE;
    }
    
    return TRUE;
}

