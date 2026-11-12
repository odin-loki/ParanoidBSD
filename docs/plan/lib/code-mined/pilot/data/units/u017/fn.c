static inline int nal_hevc_level(enum v4l2_mpeg_video_hevc_level level)
{
	/*
	 * T-Rec-H.265 p. 280: general_level_idc and sub_layer_level_idc[ i ]
	 * shall be set equal to a value of 30 times the level number
	 * specified in Table A.6.
	 */
	int factor = 30 / 10;

	switch (level) {
	case V4L2_MPEG_VIDEO_HEVC_LEVEL_1:
		return factor * 10;
	case V4L2_MPEG_VIDEO_HEVC_LEVEL_2:
		return factor * 20;
	case V4L2_MPEG_VIDEO_HEVC_LEVEL_2_1:
		return factor * 21;
	case V4L2_MPEG_VIDEO_HEVC_LEVEL_3:
		return factor * 30;
	case V4L2_MPEG_VIDEO_HEVC_LEVEL_3_1:
		return factor * 31;
	case V4L2_MPEG_VIDEO_HEVC_LEVEL_4:
		return factor * 40;
	case V4L2_MPEG_VIDEO_HEVC_LEVEL_4_1:
		return factor * 41;
	case V4L2_MPEG_VIDEO_HEVC_LEVEL_5:
		return factor * 50;
	case V4L2_MPEG_VIDEO_HEVC_LEVEL_5_1:
		return factor * 51;
	case V4L2_MPEG_VIDEO_HEVC_LEVEL_5_2:
		return factor * 52;
	case V4L2_MPEG_VIDEO_HEVC_LEVEL_6:
		return factor * 60;
	case V4L2_MPEG_VIDEO_HEVC_LEVEL_6_1:
		return factor * 61;
	case V4L2_MPEG_VIDEO_HEVC_LEVEL_6_2:
		return factor * 62;
	default:
		return -EINVAL;
	}
}
