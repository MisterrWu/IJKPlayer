/*
 * FFmpegMediaMetadataRetriever: A unified interface for retrieving frame 
 * and meta data from an input media file.
 *
 * Copyright 2016 William Seemann
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include "ffmpeg_mediametadataretriever.h"
#include "ffmpeg_utils.h"

#include <stdio.h>
#include <unistd.h>

#include "AndroidLog.h"



/*
#include <android/log.h>

#define ALOG(level, TAG, ...)    ((void)__android_log_print(level, TAG, __VA_ARGS__))
#define FFM_LOG_TAG "FFME"

#define ALOGV(...)  ALOG(ANDROID_LOG_VERBOSE,   FFM_LOG_TAG, __VA_ARGS__)
#define ALOGD(...)  ALOG(ANDROID_LOG_DEBUG,     FFM_LOG_TAG, __VA_ARGS__)
#define ALOGI(...)  ALOG(ANDROID_LOG_INFO,      FFM_LOG_TAG, __VA_ARGS__)
#define ALOGE(...)  ALOG(ANDROID_LOG_ERROR,      FFM_LOG_TAG, __VA_ARGS__)
*/

const int TARGET_IMAGE_FORMAT = AV_PIX_FMT_RGBA; //AV_PIX_FMT_RGB24;
const int TARGET_IMAGE_CODEC = AV_CODEC_ID_PNG;

void convert_image(State *state, AVCodecContext *pCodecCtx, AVFrame *pFrame, AVPacket *avpkt, int *got_packet_ptr, int width, int height);

int is_supported_format(int codec_id, int pix_fmt) {
	if ((codec_id == AV_CODEC_ID_PNG ||
		 codec_id == AV_CODEC_ID_MJPEG ||
		 codec_id == AV_CODEC_ID_BMP) &&
		pix_fmt == AV_PIX_FMT_RGBA) {
		return 1;
	}

	return 0;
}

int get_scaled_context(State *s, AVCodecContext *pCodecCtx, int width, int height) {
	AVCodec *targetCodec = avcodec_find_encoder(TARGET_IMAGE_CODEC);
	if (!targetCodec) {
		ALOGV("avcodec_find_decoder() failed to find encoder\n");
		return FAILURE;
	}

	s->scaled_codecCtx = avcodec_alloc_context3(targetCodec);
	if (!s->scaled_codecCtx) {
		ALOGV("avcodec_alloc_context3 failed\n");
		return FAILURE;
	}

	s->scaled_codecCtx->bit_rate = s->video_st->codec->bit_rate;
	s->scaled_codecCtx->width = width;
	s->scaled_codecCtx->height = height;
	s->scaled_codecCtx->pix_fmt = TARGET_IMAGE_FORMAT;
	s->scaled_codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	s->scaled_codecCtx->time_base.num = s->video_st->codec->time_base.num;
	s->scaled_codecCtx->time_base.den = s->video_st->codec->time_base.den;

	if (!targetCodec || avcodec_open2(s->scaled_codecCtx, targetCodec, NULL) < 0) {
		ALOGV("avcodec_open2() failed\n");
		return FAILURE;
	}

	s->scaled_sws_ctx = sws_getContext(s->video_st->codec->width,
			s->video_st->codec->height,
			s->video_st->codec->pix_fmt,
			width,
			height,
			TARGET_IMAGE_FORMAT,
			SWS_BILINEAR,
			NULL,
			NULL,
			NULL);

	return SUCCESS;
}

int stream_component_open(State *s, int stream_index) {
	AVFormatContext *pFormatCtx = s->pFormatCtx;
	AVCodecContext *codecCtx;
	AVCodec *codec;

	if (stream_index < 0 || stream_index >= pFormatCtx->nb_streams) {
		return FAILURE;
	}

	// Get a pointer to the codec context for the stream
	codecCtx = pFormatCtx->streams[stream_index]->codec;

    const AVCodecDescriptor *codesc = avcodec_descriptor_get(codecCtx->codec_id);
    if (codesc) {
        ALOGV("avcodec_find_decoder %s\n", codesc->name);
    }

	// Find the decoder for the audio stream
	codec = avcodec_find_decoder(codecCtx->codec_id);

	if (codec == NULL) {
	    ALOGE("avcodec_find_decoder() failed to find  decoder %s\n", codesc->name);
	    return FAILURE;
	}

	// Open the codec
    if (!codec || (avcodec_open2(codecCtx, codec, NULL) < 0)) {
	  	ALOGE("avcodec_open2() failed\n");
		return FAILURE;
	}

	switch(codecCtx->codec_type) {
		case AVMEDIA_TYPE_AUDIO:
			s->audio_stream = stream_index;
		    s->audio_st = pFormatCtx->streams[stream_index];
			break;
		case AVMEDIA_TYPE_VIDEO:
			s->video_stream = stream_index;
		    s->video_st = pFormatCtx->streams[stream_index];
                    /* cyj don't use
			AVCodec *targetCodec = avcodec_find_encoder(TARGET_IMAGE_CODEC);
			if (!targetCodec) {
			    ALOGV("avcodec_find_decoder() failed to find encoder\n");
				return FAILURE;
			}
		    

		    s->codecCtx = avcodec_alloc_context3(targetCodec);
			if (!s->codecCtx) {
				ALOGV("avcodec_alloc_context3 failed\n");
				return FAILURE;
			}

			s->codecCtx->bit_rate = s->video_st->codec->bit_rate;
			s->codecCtx->width = s->video_st->codec->width;
			s->codecCtx->height = s->video_st->codec->height;
			s->codecCtx->pix_fmt = TARGET_IMAGE_FORMAT;
			s->codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
			s->codecCtx->time_base.num = s->video_st->codec->time_base.num;
			s->codecCtx->time_base.den = s->video_st->codec->time_base.den;

			if (!targetCodec || avcodec_open2(s->codecCtx, targetCodec, NULL) < 0) {
			  	ALOGV("avcodec_open2() failed\n");
				return FAILURE;
			}

		    s->sws_ctx = sws_getContext(s->video_st->codec->width,
		    		s->video_st->codec->height,
		    		s->video_st->codec->pix_fmt,
		    		s->video_st->codec->width,
		    		s->video_st->codec->height,
		    		TARGET_IMAGE_FORMAT,
		    		SWS_BILINEAR,
		    		NULL,
		    		NULL,
		    		NULL);
			break;
		*/
		default:
			break;
	}

	return SUCCESS;
}

int set_data_source_l(State **ps, const char* path) {
	ALOGV("set_data_source\n");
	int audio_index = -1;
	int video_index = -1;
	int i;

	State *state = *ps;
	
    ALOGV("Path: %s\n", path);

    AVDictionary *options = NULL;
    av_dict_set(&options, "icy", "1", 0);
    av_dict_set(&options, "user-agent", "FFmpegMediaMetadataRetriever", 0);
    
    if (state->headers) {
        av_dict_set(&options, "headers", state->headers, 0);
    }
    
    if (state->offset > 0) {
        state->pFormatCtx = avformat_alloc_context();
        state->pFormatCtx->skip_initial_bytes = state->offset;
    }
    
    if (avformat_open_input(&state->pFormatCtx, path, NULL, &options) != 0) {
	    ALOGV("Metadata could not be retrieved\n");
		*ps = NULL;
    	return FAILURE;
    }

	if (avformat_find_stream_info(state->pFormatCtx, NULL) < 0) {
	    ALOGV("Metadata could not be retrieved\n");
	    avformat_close_input(&state->pFormatCtx);
		*ps = NULL;
    	return FAILURE;
	}

	set_duration(state->pFormatCtx);
	
	set_shoutcast_metadata(state->pFormatCtx);

	//av_dump_format(state->pFormatCtx, 0, path, 0);
	
    // Find the first audio and video stream
	for (i = 0; i < state->pFormatCtx->nb_streams; i++) {
		if (state->pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && video_index < 0) {
			video_index = i;
		}

		if (state->pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && audio_index < 0) {
			audio_index = i;
		}

		set_codec(state->pFormatCtx, i);
	}

	if (audio_index >= 0) {
		stream_component_open(state, audio_index);
	}

	if (video_index >= 0) {
		stream_component_open(state, video_index);
	}

	/*if(state->video_stream < 0 || state->audio_stream < 0) {
	    avformat_close_input(&state->pFormatCtx);
		*ps = NULL;
		return FAILURE;
	}*/

    set_rotation(state->pFormatCtx, state->audio_st, state->video_st);
    set_framerate(state->pFormatCtx, state->audio_st, state->video_st);
    set_filesize(state->pFormatCtx);
    set_chapter_count(state->pFormatCtx);
    set_video_dimensions(state->pFormatCtx, state->video_st);
    
    ALOGV("Found metadata\n");
    AVDictionaryEntry *tag = NULL;
    while ((tag = av_dict_get(state->pFormatCtx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        ALOGV("Key %s: \n", tag->key);
        ALOGV("Value %s: \n", tag->value);
    }
	
	*ps = state;
	return SUCCESS;
}

void init(State **ps) {
	State *state = *ps;

	if (state && state->pFormatCtx) {
		avformat_close_input(&state->pFormatCtx);
	}

	if (state && state->fd != -1) {
		close(state->fd);
	}
	
	if (!state) {
		state = av_mallocz(sizeof(State));
	}

	state->pFormatCtx = NULL;
	state->audio_stream = -1;
	state->video_stream = -1;
	state->audio_st = NULL;
	state->video_st = NULL;
	state->fd = -1;
	state->offset = 0;
	state->headers = NULL;

	*ps = state;
	av_log_set_callback(ffp_log_callback_report); 
}

int set_data_source_uri(State **ps, const char* path, const char* headers) {
	State *state = *ps;
	
	ANativeWindow *native_window = NULL;

	if (state && state->native_window) {
		native_window = state->native_window;
	}

	init(&state);
	
	state->native_window = native_window;

	state->headers = headers;
	
	*ps = state;
	
	return set_data_source_l(ps, path);
}

int set_data_source_fd(State **ps, int fd, int64_t offset, int64_t length) {
    char path[256] = "";

	State *state = *ps;
	
	ANativeWindow *native_window = NULL;

	if (state && state->native_window) {
		native_window = state->native_window;
	}

	init(&state);

	state->native_window = native_window;
    	
    int myfd = dup(fd);

    char str[20];
    sprintf(str, "pipe:%d", myfd);
    strcat(path, str);
    
    state->fd = myfd;
    state->offset = offset;
    
	*ps = state;
    
    return set_data_source_l(ps, path);
}

const char* extract_metadata(State **ps, const char* key) {
	ALOGV("extract_metadata\n");
    char* value = NULL;
	
	State *state = *ps;
    
	if (!state || !state->pFormatCtx) {
		return value;
	}

	return extract_metadata_internal(state->pFormatCtx, state->audio_st, state->video_st, key);
}

const char* extract_metadata_from_chapter(State **ps, const char* key, int chapter) {
	ALOGV("extract_metadata_from_chapter\n");
    char* value = NULL;
	
	State *state = *ps;
	
	if (!state || !state->pFormatCtx || state->pFormatCtx->nb_chapters <= 0) {
		return value;
	}

	if (chapter < 0 || chapter >= state->pFormatCtx->nb_chapters) {
		return value;
	}

	return extract_metadata_from_chapter_internal(state->pFormatCtx, state->audio_st, state->video_st, key, chapter);
}

int get_metadata(State **ps, AVDictionary **metadata) {
    ALOGV("get_metadata\n");
    
    State *state = *ps;
    
    if (!state || !state->pFormatCtx) {
        return FAILURE;
    }
    
    get_metadata_internal(state->pFormatCtx, metadata);
    
    return SUCCESS;
}

int get_embedded_picture(State **ps, AVPacket *pkt) {
	ALOGV("get_embedded_picture\n");
	int i = 0;
	int got_packet = 0;
	AVFrame *frame = NULL;
	
	State *state = *ps;

	if (!state || !state->pFormatCtx) {
		return FAILURE;
	}

    // TODO commented out 5/31/16, do we actully need this since the context
    // has been initialized
    // read the format headers
    /*if (state->pFormatCtx->iformat->read_header(state->pFormatCtx) < 0) {
    	ALOGV("Could not read the format header\n");
    	return FAILURE;
    }*/

    // find the first attached picture, if available
    for (i = 0; i < state->pFormatCtx->nb_streams; i++) {
        if (state->pFormatCtx->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC) {
        	ALOGV("Found album art\n");
        	if (pkt) {
        		av_packet_unref(pkt);
        		av_init_packet(pkt);
        	}
            av_copy_packet(pkt, &state->pFormatCtx->streams[i]->attached_pic);
			// TODO is this right
			got_packet = 1;
        	
        	// Is this a packet from the video stream?
        	if (pkt->stream_index == state->video_stream) {
        		int codec_id = state->video_st->codec->codec_id;
				int pix_fmt = state->video_st->codec->pix_fmt;

        		// If the image isn't already in a supported format convert it to one
        		if (!is_supported_format(codec_id, pix_fmt)) {
        			int got_frame = 0;
        			
   			        frame = av_frame_alloc();
        			    	
   			        if (!frame) {
   			        	break;
        			}
   			        
        			if (avcodec_decode_video2(state->video_st->codec, frame, &got_frame, pkt) <= 0) {
        				break;
        			}

        			// Did we get a video frame?
        			if (got_frame) {
        				AVPacket convertedPkt;
        	            av_init_packet(&convertedPkt);
        	            convertedPkt.size = 0;
        	            convertedPkt.data = NULL;

        	            convert_image(state, state->video_st->codec, frame, &convertedPkt, &got_packet, -1, -1);

        				av_packet_unref(pkt);
        				av_init_packet(pkt);
        				av_copy_packet(pkt, &convertedPkt);

        				av_packet_unref(&convertedPkt);

        				break;
        			}
        		} else {
        			av_packet_unref(pkt);
                	av_init_packet(pkt);
                    av_copy_packet(pkt, &state->pFormatCtx->streams[i]->attached_pic);
        			
        			got_packet = 1;
        			break;
        		}
        	}
        }
    }

	av_frame_free(&frame);

	if (got_packet) {
		return SUCCESS;
	} else {
		return FAILURE;
	}
}

void convert_image(State *state, AVCodecContext *pCodecCtx, AVFrame *pFrame, AVPacket *avpkt, int *got_packet_ptr, int width, int height) {
	AVCodecContext *codecCtx;
	struct SwsContext *scalerCtx;
	AVFrame *frame;
	
	*got_packet_ptr = 0;

	if (width != -1 && height != -1) {
		if (state->scaled_codecCtx == NULL ||
				state->scaled_sws_ctx == NULL) {
			get_scaled_context(state, pCodecCtx, width, height);
		}

		codecCtx = state->scaled_codecCtx;
		scalerCtx = state->scaled_sws_ctx;
	} else {
		codecCtx = state->codecCtx;
		scalerCtx = state->sws_ctx;
	}

	if (width == -1) {
		width = pCodecCtx->width;
	}

	if (height == -1) {
		height = pCodecCtx->height;
	}

	frame = av_frame_alloc();
	
	// Determine required buffer size and allocate buffer
	ALOGV("convert_image codecCtx=%p", codecCtx);
	int numBytes = avpicture_get_size(TARGET_IMAGE_FORMAT, codecCtx->width, codecCtx->height);
	void * buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    ALOGV("convert_image buffer=%p", buffer);
    
	// set the frame parameters
	frame->format = TARGET_IMAGE_FORMAT;
	frame->width = codecCtx->width;
	frame->height = codecCtx->height;

	avpicture_fill(((AVPicture *)frame),
			buffer,
			TARGET_IMAGE_FORMAT,
			codecCtx->width,
			codecCtx->height);
    
    sws_scale(scalerCtx,
    		(const uint8_t * const *) pFrame->data,
    		pFrame->linesize,
    		0,
    		pFrame->height,
    		frame->data,
            frame->linesize);

	int ret = avcodec_encode_video2(codecCtx, avpkt, frame, got_packet_ptr);
	
	if (ret >= 0 && state->native_window) {
		ANativeWindow_setBuffersGeometry(state->native_window, width, height, WINDOW_FORMAT_RGBA_8888);

		ANativeWindow_Buffer windowBuffer;

		if (ANativeWindow_lock(state->native_window, &windowBuffer, NULL) == 0) {
			//__android_log_print(ANDROID_LOG_VERBOSE, "LOG_TAG", "width %d", windowBuffer.width);
			//__android_log_print(ANDROID_LOG_VERBOSE, "LOG_TAG", "height %d", windowBuffer.height);

			int h = 0;

			for (h = 0; h < height; h++)  {
			  memcpy(windowBuffer.bits + h * windowBuffer.stride * 4,
			         buffer + h * frame->linesize[0],
			         width*4);
			}

			ANativeWindow_unlockAndPost(state->native_window);
		}
	}
	
	if (ret < 0) {
		*got_packet_ptr = 0;
	}
	
    av_frame_free(&frame);
	
    if (buffer) {
    	free(buffer);
    }

	if (ret < 0 || !*got_packet_ptr) {
		av_packet_unref(avpkt);
	}
}

void decode_frame(State *state, AVPacket *pkt, int *got_frame, int64_t desired_frame_number, int width, int height) {
	// Allocate video frame
	AVFrame *frame = av_frame_alloc();

	*got_frame = 0;
	
	if (!frame) {
	    return;
	}

	// Read frames and return the first one found
	while (av_read_frame(state->pFormatCtx, pkt) >= 0) {

		// Is this a packet from the video stream?
		if (pkt->stream_index == state->video_stream) {
			int codec_id = state->video_st->codec->codec_id;
			int pix_fmt = state->video_st->codec->pix_fmt;
			
			// If the image isn't already in a supported format convert it to one
			if (!is_supported_format(codec_id, pix_fmt)) {
	            *got_frame = 0;
	            
				// Decode video frame
				if (avcodec_decode_video2(state->video_st->codec, frame, got_frame, pkt) <= 0) {
					*got_frame = 0;
					break;
				}

				// Did we get a video frame?
				if (*got_frame) {
					if (desired_frame_number == -1 ||
							(desired_frame_number != -1 && frame->pkt_pts >= desired_frame_number)) {
						if (pkt->data) {
							av_packet_unref(pkt);
						}
					    av_init_packet(pkt);
						convert_image(state, state->video_st->codec, frame, pkt, got_frame, width, height);
						break;
					}
				}
			} else {
				*got_frame = 1;
	        	break;
			}
		}
	}
	
	// Free the frame
	av_frame_free(&frame);
}

int get_frame_at_time(State **ps, int64_t timeUs, int option, AVPacket *pkt) {
	return get_scaled_frame_at_time(ps, timeUs, option, pkt, -1, -1);
}

int get_scaled_frame_at_time(State **ps, int64_t timeUs, int option, AVPacket *pkt, int width, int height) {
	ALOGV("get_frame_at_time\n");
	int got_packet = 0;
    int64_t desired_frame_number = -1;
	
    State *state = *ps;

    Options opt = option;
    
	if (!state || !state->pFormatCtx || state->video_stream < 0) {
		return FAILURE;
	}
	
    if (timeUs != -1) {
        int stream_index = state->video_stream;
        int64_t seek_time = av_rescale_q(timeUs, AV_TIME_BASE_Q, state->pFormatCtx->streams[stream_index]->time_base);
        int64_t seek_stream_duration = state->pFormatCtx->streams[stream_index]->duration;

        int flags = 0;
        int ret = -1;
        
        // For some reason the seek_stream_duration is sometimes a negative value,
        // make sure to check that it is greater than 0 before adjusting the
        // seek_time
        if (seek_stream_duration > 0 && seek_time > seek_stream_duration) {
        	seek_time = seek_stream_duration;
        }
        
        if (seek_time < 0) {
        	return FAILURE;
       	}
        
        if (opt == OPTION_CLOSEST) {
        	desired_frame_number = seek_time;
        	flags = AVSEEK_FLAG_BACKWARD; 
        } else if (opt == OPTION_CLOSEST_SYNC) {
        	flags = 0;
        } else if (opt == OPTION_NEXT_SYNC) {
        	flags = 0;
        } else if (opt == OPTION_PREVIOUS_SYNC) {
        	flags = AVSEEK_FLAG_BACKWARD;
        }
        
        ret = av_seek_frame(state->pFormatCtx, stream_index, seek_time, flags);
        
    	if (ret < 0) {
    		return FAILURE;
    	} else {
            if (state->audio_stream >= 0) {
            	avcodec_flush_buffers(state->audio_st->codec);
            }
    		
            if (state->video_stream >= 0) {
            	avcodec_flush_buffers(state->video_st->codec);
            }
    	}
    }
    
    decode_frame(state, pkt, &got_packet, desired_frame_number, width, height);
    
    if (got_packet) {
    	//const char *filename = "/Users/wseemann/Desktop/one.png";
    	//FILE *picture = fopen(filename, "wb");
    	//fwrite(pkt->data, pkt->size, 1, picture);
    	//fclose(picture);
    }
    
	if (got_packet) {
		return SUCCESS;
	} else {
		return FAILURE;
	}
}

/* cyj  use AVFrame */

int scaleToRgb565(enum AVPixelFormat srcFmt, AVFrame *srcFrm, VideoFrame *dstFrm, int dst_w, int dst_h){
    int src_w = srcFrm->width, src_h = srcFrm->height;
    enum AVPixelFormat src_pix_fmt = srcFmt, dst_pix_fmt = AV_PIX_FMT_RGB565;
    struct SwsContext *sws_ctx;
    int ret = 0;
    
    dstFrm->width = 0;
    dstFrm->height = 0;
    if(dst_w <= 10 ||dst_h <= 10){
        ALOGE( "scaleToRgb565 dst_w=%d, dst_h=%d error", dst_w,  dst_h);
        return -1;
    }
    if(src_w > src_h){
        dst_h = dst_w * src_h /src_w;
    }else if(src_w < src_h){
        dst_w = dst_h * src_w /src_h;
    }
    
    
    /* create scaling context */
    sws_ctx = sws_getContext(src_w, src_h, src_pix_fmt,
                             dst_w, dst_h, dst_pix_fmt,
                             SWS_FAST_BILINEAR, NULL, NULL, NULL);
    if (!sws_ctx) {
        /*ALOGE(
                "Impossible to create scale context for the conversion "
                "fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
                av_get_pix_fmt_name(src_pix_fmt), src_w, src_h,
                av_get_pix_fmt_name(dst_pix_fmt), dst_w, dst_h);*/
        ret = AVERROR(EINVAL);
        return ret;
    }
    /*ALOGV(
            "create scale context for the conversion "
            "fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
            av_get_pix_fmt_name(src_pix_fmt), src_w, src_h,
            av_get_pix_fmt_name(dst_pix_fmt), dst_w, dst_h);*/

    /* allocate source and destination image buffers */
    if ((ret = av_image_alloc(dstFrm->data, dstFrm->linesize,
                              dst_w, dst_h, dst_pix_fmt, 32)) < 0) {
        ALOGV( "Could not allocate dest image\n");
        ret = -1;
        goto end;
    }
    ALOGV( "sws_isSupportedOutput  rgb32 %d\n",sws_isSupportedOutput(AV_PIX_FMT_RGB32));
    ALOGV( "dest image linesize[0]=%d\n", dstFrm->linesize[0]);
    ALOGV( "src image linesize[0]=%d\n", srcFrm->linesize[0]);
    ALOGV( "src image linesize[1]=%d\n", srcFrm->linesize[1]);
    ALOGV( "src image linesize[2]=%d\n", srcFrm->linesize[2]);
    /* convert to destination format */
    dstFrm->height = sws_scale(sws_ctx, (const uint8_t * const*)srcFrm->data,
                                                srcFrm->linesize, 0, srcFrm->height, dstFrm->data, dstFrm->linesize);
    dstFrm->width = dst_w;
    if (1) {
    	const char *filename = "/sdcard/one.yuv";
    	FILE *picture = fopen(filename, "wb");        
        fwrite(srcFrm->data[0], srcFrm->linesize[0]*src_h, 1, picture);
        fwrite(srcFrm->data[1], srcFrm->linesize[1]*src_h/4, 1, picture);
        fwrite(srcFrm->data[2], srcFrm->linesize[2]*src_h/4, 1, picture);
        fclose(picture);
        

    }
    ALOGV( "dest image width=%d, height=%d\n", dstFrm->width, dstFrm->height);
    if(dstFrm->height <= 0){
        ret =  -2;
        av_freep(&(dstFrm->data[0]));
    }
end:
    sws_freeContext(sws_ctx);
    return     ret;
}

int scaleToRgb(enum AVPixelFormat srcFmt, AVFrame *srcFrm, VideoFrame *dstFrm, int dst_w, int dst_h){
    if(srcFmt == AV_PIX_FMT_YUV420P){
        /*I420ToRGBA(srcFrm->data[0], srcFrm->linesize[0],
                       srcFrm->data[1], srcFrm->linesize[1],
                       srcFrm->data[2], srcFrm->linesize[2],
                       dstFrm->data[0], dstFrm->linesize[0],
                       srcFrm->width, srcFrm->height);*/
        
    }

}


void decode_frame2(State *state, VideoFrame *frm, int *got_frame, int64_t desired_frame_number, int width, int height) {
    ALOGV("decode_frame2\n");
    // Allocate video frame
    AVFrame *frame = av_frame_alloc();
    AVPacket pkt;
    *got_frame = 0;
    
    if (!frame) {
        ALOGE("av_frame_alloc() failed!!!\n");
        return;
    }
    av_init_packet(&pkt);
	int64_t mFirstKeyPktTimestamp = AV_NOPTS_VALUE;
    // Read frames and return the first one found
    while (av_read_frame(state->pFormatCtx, &pkt) >= 0) {
        ALOGV( "av read frame \n");
        // Is this a packet from the video stream?
        if (pkt.stream_index == state->video_stream) {
            int codec_id = state->video_st->codec->codec_id;
            int pix_fmt = state->video_st->codec->pix_fmt;


			
			if (pkt.pts != AV_NOPTS_VALUE && mFirstKeyPktTimestamp == AV_NOPTS_VALUE) {
				// update the first key timestamp
				mFirstKeyPktTimestamp = pkt.pts;
				ALOGD(" mFirstKeyPktTimestamp %lld", mFirstKeyPktTimestamp);
			}
			if (pkt.pts != AV_NOPTS_VALUE && pkt.pts < mFirstKeyPktTimestamp) {
				ALOGD("drop the packet with the backward timestamp, maybe they are B-frames after I-frame ^_^ %lld", pkt.pts);
				//buffer->release();
				//buffer = NULL;
				continue;
			}

							
            // Decode video frame
            if (avcodec_decode_video2(state->video_st->codec, frame, got_frame, &pkt) <= 0) {
                ALOGE("avcodec_decode_video2 failed!!!\n");
                *got_frame = 0;
                break;
            }

            // Did we get a video frame?
            if (*got_frame) {
                if (desired_frame_number == -1 ||
                        (desired_frame_number != -1 && frame->pkt_pts >= desired_frame_number)) {

                   if (scaleToRgb565(pix_fmt, frame, frm, width, height) < 0) *got_frame = 0;
                    break;
                }
            }
            
        }
        
    }
    av_packet_unref(&pkt);
    // Free the frame
    av_frame_free(&frame);
}

int get_frame_at_time2(State **ps, int64_t timeUs, int option, VideoFrame *frm) {
	return get_scaled_frame_at_time(ps, timeUs, option, frm, -1, -1);
}

int get_scaled_frame_at_time2(State **ps, int64_t timeUs, int option, VideoFrame *frm, int width, int height) {
	ALOGV("get_scaled_frame_at_time2\n");
	int got_frame = 0;
    int64_t desired_frame_number = -1;
	
    State *state = *ps;

    Options opt = option;
    
	if (!state || !state->pFormatCtx || state->video_stream < 0) {
	        ALOGE("no context  error !!!\n");
		return FAILURE;
	}
    /* cyj begin */
    if(timeUs == -1) {
        if (state->pFormatCtx->duration != AV_NOPTS_VALUE) {
		timeUs = state->pFormatCtx->duration / AV_TIME_BASE* 1000*1000 /4;
	}
    }
    /* cyj end */
    ALOGV("timeUs = %lld    duration = %lld\n", timeUs, state->pFormatCtx->duration);
    if (timeUs != -1) {
        int stream_index = state->video_stream;
        int64_t seek_time = av_rescale_q(timeUs, AV_TIME_BASE_Q, state->pFormatCtx->streams[stream_index]->time_base);
        int64_t seek_stream_duration = state->pFormatCtx->streams[stream_index]->duration;

        int flags = 0;
        int ret = -1;
        
        // For some reason the seek_stream_duration is sometimes a negative value,
        // make sure to check that it is greater than 0 before adjusting the
        // seek_time
        if (seek_stream_duration > 0 && seek_time > seek_stream_duration) {
        	seek_time = seek_stream_duration;
        }
        ALOGV("seek_time = %lld    seek_stream_duration = %lld\n", seek_time, seek_stream_duration);
        if (seek_time < 0) {
            ALOGE("seek_time  error !!!\n");
        	return FAILURE;
       	}
        
        if (opt == OPTION_CLOSEST) {
        	desired_frame_number = seek_time;
        	flags = AVSEEK_FLAG_BACKWARD; 
        } else if (opt == OPTION_CLOSEST_SYNC) {
        	flags = 0;
        } else if (opt == OPTION_NEXT_SYNC) {
        	flags = 0;
        } else if (opt == OPTION_PREVIOUS_SYNC) {
        	flags = AVSEEK_FLAG_BACKWARD;
        }
        
        ret = av_seek_frame(state->pFormatCtx, stream_index, seek_time, flags);
        
    	if (ret < 0) {
    	        ALOGE("av_seek_frame error !!!\n");
    		return FAILURE;
    	} else {
            if (state->audio_stream >= 0) {
            	avcodec_flush_buffers(state->audio_st->codec);
            }
    		
            if (state->video_stream >= 0) {
            	avcodec_flush_buffers(state->video_st->codec);
            }
    	}
    }
    
    decode_frame2(state, frm, &got_frame, desired_frame_number, width, height);
    
    
	if (got_frame) {
		return SUCCESS;
	} else {
		return FAILURE;
	}
}

/* cyj end*/
int set_native_window(State **ps, ANativeWindow* native_window) {
    ALOGV("set_native_window\n");

	State *state = *ps;

	if (native_window == NULL) {
		return FAILURE;
	}

	if (!state) {
		init(&state);
	}

	state->native_window = native_window;

	*ps = state;

	return SUCCESS;
}

void release(State **ps) {
	ALOGV("release\n");

	State *state = *ps;
	
    if (state) {
        if (state->audio_st && state->audio_st->codec) {
            avcodec_close(state->audio_st->codec);
        }
        
        if (state->video_st && state->video_st->codec) {
            avcodec_close(state->video_st->codec);
        }
        
        if (state->pFormatCtx) {
    		avformat_close_input(&state->pFormatCtx);
    	}
    	
    	if (state->fd != -1) {
    		close(state->fd);
    	}
    	
    	if (state->sws_ctx) {
    		sws_freeContext(state->sws_ctx);
    		state->sws_ctx = NULL;
	    }

    	if (state->codecCtx) {
    		avcodec_close(state->codecCtx);
    	    av_free(state->codecCtx);
    	}

    	if (state->sws_ctx) {
    		sws_freeContext(state->sws_ctx);
    	}

    	if (state->scaled_codecCtx) {
    		avcodec_close(state->scaled_codecCtx);
    	    av_free(state->scaled_codecCtx);
    	}

    	if (state->scaled_sws_ctx) {
    		sws_freeContext(state->scaled_sws_ctx);
    	}

        // make sure we don't leak native windows
        if (state->native_window != NULL) {
            ANativeWindow_release(state->native_window);
            state->native_window = NULL;
        }

    	av_freep(&state);
        ps = NULL;
    }
}
