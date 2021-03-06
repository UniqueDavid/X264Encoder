#include<stdio.h>
#include<stdlib.h>

#include "stdint.h"

#if defined (__cplusplus)
extern "C"
{
#include"x264.h"
}
#else
#include"x264.h"
#endif

int main(int argc,char** argv)
{
	int ret;
	int y_size;
	int i,j;
	int frame_num =0;
	int width=1024,height=600;
	int csp=X264_CSP_I420;

	FILE *fp_src = fopen("../1.yuv","rb");
	FILE *fp_des = fopen("../output.h264","wb");

	//��ʼ����
	int iNal = 0;
	x264_nal_t *pNals = NULL;
	x264_t *phandle = NULL;
	x264_picture_t *pPic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));
	x264_picture_t *pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));
	x264_param_t *pParam = (x264_param_t*)malloc(sizeof(x264_param_t));

	if(fp_src==NULL||fp_des==NULL)
	{
		printf("Error open files!!\n");
		return -1;
	}

	x264_param_default(pParam);
	pParam->i_width = width;
	pParam->i_height = height;
	pParam->i_csp = csp;

	x264_param_apply_profile(pParam, x264_profile_names[5]);

	phandle = x264_encoder_open(pParam);

	x264_picture_init(pPic_out);
	x264_picture_alloc(pPic_in,csp,pParam->i_width,pParam->i_height);

	y_size = pParam->i_width*pParam->i_height;

	fseek(fp_src, 0, SEEK_END);
	switch (csp)
	{
	case X264_CSP_I420:
		frame_num = ftell(fp_src)/(y_size*3/2);
		break;
	default:
		printf("csp colorspace not support!!\n");
		return -1;
	}
	fseek(fp_src,0,SEEK_SET);

	//Loop to encoder
	for (i=0;i<frame_num;i++)
	{
		switch (csp)
		{
		case X264_CSP_I420:
			fread(pPic_in->img.plane[0],y_size,1,fp_src);//Y
			fread(pPic_in->img.plane[1],y_size/4,1,fp_src);//U
			fread(pPic_in->img.plane[2],y_size/4,1,fp_src);//V
			break;
		default:
			{
				printf("colorspace not support!!\n");
				return -1;
			}
		}

		pPic_in->i_pts = i;

		ret = x264_encoder_encode(phandle,&pNals,&iNal, pPic_in, pPic_out);

		if(ret<0)
		{
			printf("Error!!\n");
			return -1;
		}

		printf("Succeed encode frame:%5d\n",i);
		for(j=0;j<iNal;++j)
		{
			fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_des);
		}
	}
	i=0;
	//flush encoder
	while(1)
	{
		ret = x264_encoder_encode(phandle,&pNals,&iNal,NULL,pPic_out);
		if (ret == 0)
			break;
		printf("Flush 1 frame.\n");
		for(j=0;j<iNal;++j)
		{
			fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_des);
		}
		i++;
	}

	//clean
	x264_picture_clean(pPic_in);
	x264_encoder_close(phandle);
	phandle=NULL;

	free(pPic_in);
	free(pPic_out);
	free(pParam);

	fclose(fp_src);
	fclose(fp_des);
		
	return 0;
}