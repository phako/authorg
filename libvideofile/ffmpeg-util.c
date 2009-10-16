/* Authorg - a GNOME DVD authoring tool
 *
 * Copyright (C) 2005 Jens Georg <mail@jensge.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 */


#include <config.h>

#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>

#include <glib/gerror.h>
#include <glib/gi18n.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "ffmpeg-util.h"

/**
 * authorg_ffmpeg_error_to_string:
 * @error: an FFMPEG error code
 *
 * Converts a FFMPEG error code to a human readable message.
 * Returns: a newly allocated string
 */
gchar *
authorg_ffmpeg_util_error_to_string (gint error)
{
	switch (error)
	{
		case AVERROR_IO:
			return g_strdup (_("I/O error"));
		case AVERROR_NUMEXPECTED:
			return g_strdup (_("Number syntax expected in filename"));
		case AVERROR_INVALIDDATA:
			return g_strdup (_("Invalid data found"));
		case AVERROR_NOMEM:
			return g_strdup (_("Not enough memory"));
		case AVERROR_NOFMT:
			return g_strdup (_("Unknown format"));
		case AVERROR_NOTSUPP:
			return g_strdup (_("Operation not supported"));

		default:
			return g_strdup (_("Unknown error"));
	}
}

GQuark
authorg_ffmpeg_util_quark ()
{
	static GQuark q = 0;
	if (q == 0)
		q = g_quark_from_static_string ("authorg-ffmpeg-error-quark");
	return q;
}

/**
 * authorg_ffmpeg_grab_pixbuf:
 * @filename: the video file to grab from
 * @timecode: time to grab in seconds
 * @error: a pointer to a GError * or NULL if you do not want to receive error messages
 *
 * Grabs a picture from a video into a GdkPixbuf
 * Returns: A GdkPixbuf or NULL on error
 */
GdkPixbuf *
authorg_ffmpeg_grab_pixbuf (const gchar *filename, guint timecode, GError **error)
{
	int             i, videoStream;
	AVFormatContext *pFormatCtx = NULL;
	AVCodecContext  *pCodecCtx = NULL;
	AVCodec         *pCodec;
	AVFrame         *pFrame = NULL; 
	AVFrame         *pFrameRGB = NULL;
	AVPacket        packet;
	gint             frameFinished;
	gint             numBytes;
	uint8_t         *buffer = NULL;
	GdkPixbuf		*pixbuf = NULL;
	gint			res;
	gchar *tmp;

	g_return_val_if_fail (error != NULL || *error != NULL, NULL);

	/* Register all formats and codecs */
	av_register_all ();

	/* Open video file */
	if ((res = av_open_input_file(&pFormatCtx, filename, NULL, 0, NULL)) != 0)
	{
		g_set_error (error, 
				AUTHORG_FFMPEG_ERROR,
				AUTHORG_FFMPEG_ERROR_OPEN,
				"Could not open file: %s",
				tmp = authorg_ffmpeg_util_error_to_string (res));
		g_free (tmp);
		goto cleanup;
	}

	/* Retrieve stream information */
	if ((res = av_find_stream_info (pFormatCtx)) < 0)
	{
		g_set_error (error, 
				AUTHORG_FFMPEG_ERROR,
				AUTHORG_FFMPEG_ERROR_INFO,
				"Could get stream information: %s",
				tmp = authorg_ffmpeg_util_error_to_string (res));
		g_free (tmp);
		goto cleanup;
	}

	/* Find the first video stream */
	videoStream=-1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
		{
			videoStream=i;
			break;
		}
	
	if (videoStream == -1)
	{
		g_set_error (error, 
				AUTHORG_FFMPEG_ERROR,
				AUTHORG_FFMPEG_ERROR_NO_VIDEO,
				"Stream does not contain video data");

		goto cleanup;
	}

	/* Get a pointer to the codec context for the video stream */
	pCodecCtx = pFormatCtx->streams[videoStream]->codec;

	/* Find the decoder for the video stream */
	pCodec = avcodec_find_decoder (pCodecCtx->codec_id);
	if(pCodec==NULL)
	{
		g_set_error (error, 
				AUTHORG_FFMPEG_ERROR,
				AUTHORG_FFMPEG_ERROR_NO_VIDEO,
				"Could not find codec for video stream");

		goto cleanup;
	}

	/* Open codec */
	if ((res = avcodec_open (pCodecCtx, pCodec)) < 0)
	{
		g_set_error (error, 
				AUTHORG_FFMPEG_ERROR,
				AUTHORG_FFMPEG_ERROR_INFO,
				"Could not open codec: %s",
				tmp = authorg_ffmpeg_util_error_to_string (res));

		g_free (tmp);
		goto cleanup;
	}

	/* Hack to correct wrong frame rates that seem to be generated by some 
	 codecs */
/*	if (pCodecCtx->frame_rate > 1000 && pCodecCtx->frame_rate_base == 1)
		pCodecCtx->frame_rate_base = 1000;
*/
	/* Allocate video frame */
	pFrame = avcodec_alloc_frame();
	if (pFrame == NULL)
	{
		g_set_error (error, 
				AUTHORG_FFMPEG_ERROR,
				AUTHORG_FFMPEG_ERROR_NO_FRAME,
				"Could not allocate video frame");

		goto cleanup;
	}

	/* Allocate an AVFrame structure */
	pFrameRGB = avcodec_alloc_frame();
	if (pFrameRGB == NULL)
	{
		g_set_error (error, 
				AUTHORG_FFMPEG_ERROR,
				AUTHORG_FFMPEG_ERROR_NO_FRAME,
				"Could not allocate video frame");

		goto cleanup;
	}

	/* Determine required buffer size and allocate buffer */
	numBytes = avpicture_get_size(PIX_FMT_RGB24,
			pCodecCtx->width, pCodecCtx->height); 
	buffer = g_new0 (uint8_t, numBytes);

	/* Assign appropriate parts of buffer to image planes in pFrameRGB */
	avpicture_fill ((AVPicture *)pFrameRGB,
			buffer, 
			PIX_FMT_RGB24,
			pCodecCtx->width, 
			pCodecCtx->height);

	/* Seek to given frame */
	av_seek_frame (pFormatCtx, -1, timecode * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);

	/* Read frame, find video frame and build pixbuf */
	while (av_read_frame (pFormatCtx, &packet) >= 0)
	{
		/* Is this a packet from the video stream? */
		if (packet.stream_index == videoStream)
		{
			/* Decode video frame */
			avcodec_decode_video (pCodecCtx, 
					pFrame, &frameFinished, 
					packet.data, packet.size);

			/* Did we get a video frame? */
			if(frameFinished)
			{
				/* Convert the image from its native format to RGB */
				img_convert((AVPicture *)pFrameRGB, PIX_FMT_RGB24, 
						(AVPicture*)pFrame, pCodecCtx->pix_fmt, pCodecCtx->width, 
						pCodecCtx->height);

				/* create pixbuf */
				pixbuf = gdk_pixbuf_new_from_data (
						pFrameRGB->data[0],
						GDK_COLORSPACE_RGB,
						FALSE,
						8,
						pCodecCtx->width,
						pCodecCtx->height,
						pFrameRGB->linesize[0],
						NULL, NULL);
				break;
			}
		}

		/* Free the packet that was allocated by av_read_frame */
		av_free_packet(&packet);
	}

cleanup:
	/* it is safe to free this, but buffer mustn't be freed */
	if (pFrameRGB)
	    av_free(pFrameRGB);

	if (pFrame)
	    av_free(pFrame);

	if (pCodecCtx)
	    avcodec_close(pCodecCtx);

	if (pFormatCtx)
	    av_close_input_file(pFormatCtx);
	
	return pixbuf;
}
