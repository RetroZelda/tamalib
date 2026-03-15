#ifndef OPCODE_LIST

/* The E0C6S46 supported instructions for use in an X Macro*/
#define OPCODE_LIST\
/*	 				log							    Code	mask		shift_arg0	mask_arg0	cycles	func			*/ \
/*	PSET		*/	X(LOG_OPCODE_PSET,		0xE40,	MASK_7B,	0, 			0    , 		5 , 	&op_pset_cb		)\
/*	JP			*/	X(LOG_OPCODE_JP,		0x000,	MASK_4B,	0, 			0    , 		5 , 	&op_jp_cb		)\
/*	JP_C		*/	X(LOG_OPCODE_JP_C,		0x200,	MASK_4B,	0, 			0    , 		5 , 	&op_jp_c_cb		)\
/*	JP_NC		*/	X(LOG_OPCODE_JP_NC,		0x300,	MASK_4B,	0, 			0    , 		5 , 	&op_jp_nc_cb	)\
/*	JP_Z		*/	X(LOG_OPCODE_JP_Z,		0x600,	MASK_4B,	0, 			0    , 		5 , 	&op_jp_z_cb		)\
/*	JP_NZ		*/	X(LOG_OPCODE_JP_NZ,		0x700,	MASK_4B,	0, 			0    , 		5 , 	&op_jp_nz_cb	)\
/*	JPBA		*/	X(LOG_OPCODE_JPBA,		0xFE8,	MASK_12B,	0, 			0    , 		5 , 	&op_jpba_cb		)\
/*	CALL		*/	X(LOG_OPCODE_CALL,		0x400,	MASK_4B,	0, 			0    , 		7 , 	&op_call_cb		)\
/*	CALZ		*/	X(LOG_OPCODE_CALZ,		0x500,	MASK_4B,	0, 			0    , 		7 , 	&op_calz_cb		)\
/*	RET			*/	X(LOG_OPCODE_RET,		0xFDF,	MASK_12B,	0, 			0    , 		7 , 	&op_ret_cb		)\
/*	RETS		*/	X(LOG_OPCODE_RETS,		0xFDE,	MASK_12B,	0, 			0    , 		12, 	&op_rets_cb		)\
/*	RETD		*/	X(LOG_OPCODE_RETD,		0x100,	MASK_4B,	0, 			0    , 		12, 	&op_retd_cb		)\
/*	NOP5		*/	X(LOG_OPCODE_NOP5,		0xFFB,	MASK_12B,	0, 			0    , 		5 , 	&op_nop5_cb		)\
/*	NOP7		*/	X(LOG_OPCODE_NOP7,		0xFFF,	MASK_12B,	0, 			0    , 		7 , 	&op_nop7_cb		)\
/*	HALT		*/	X(LOG_OPCODE_HALT,		0xFF8,	MASK_12B,	0, 			0    , 		5 , 	&op_halt_cb		)\
/*	INC_X		*/	X(LOG_OPCODE_INC_X,		0xEE0,	MASK_12B,	0, 			0    , 		5 , 	&op_inc_x_cb	)\
/*	INC_Y		*/	X(LOG_OPCODE_INC_Y,		0xEF0,	MASK_12B,	0, 			0    , 		5 , 	&op_inc_y_cb	)\
/*	LD_X		*/	X(LOG_OPCODE_LD_X,		0xB00,	MASK_4B,	0, 			0    , 		5 , 	&op_ld_x_cb		)\
/*	LD_Y		*/	X(LOG_OPCODE_LD_Y,		0x800,	MASK_4B,	0, 			0    , 		5 , 	&op_ld_y_cb		)\
/*	LD_XP_R		*/	X(LOG_OPCODE_LD_XP_R,	0xE80,	MASK_10B,	0, 			0    , 		5 , 	&op_ld_xp_r_cb	)\
/*	LD_XH_R		*/	X(LOG_OPCODE_LD_XH_R,	0xE84,	MASK_10B,	0, 			0    , 		5 , 	&op_ld_xh_r_cb	)\
/*	LD_XL_R		*/	X(LOG_OPCODE_LD_XL_R,	0xE88,	MASK_10B,	0, 			0    , 		5 , 	&op_ld_xl_r_cb	)\
/*	LD_YP_R		*/	X(LOG_OPCODE_LD_YP_R,	0xE90,	MASK_10B,	0, 			0    , 		5 , 	&op_ld_yp_r_cb	)\
/*	LD_YH_R		*/	X(LOG_OPCODE_LD_YH_R,	0xE94,	MASK_10B,	0, 			0    , 		5 , 	&op_ld_yh_r_cb	)\
/*	LD_YL_R		*/	X(LOG_OPCODE_LD_YL_R,	0xE98,	MASK_10B,	0, 			0    , 		5 , 	&op_ld_yl_r_cb	)\
/*	LD_R_XP		*/	X(LOG_OPCODE_LD_R_XP,	0xEA0,	MASK_10B,	0, 			0    , 		5 , 	&op_ld_r_xp_cb	)\
/*	LD_R_XH		*/	X(LOG_OPCODE_LD_R_XH,	0xEA4,	MASK_10B,	0, 			0    , 		5 , 	&op_ld_r_xh_cb	)\
/*	LD_R_XL		*/	X(LOG_OPCODE_LD_R_XL,	0xEA8,	MASK_10B,	0, 			0    , 		5 , 	&op_ld_r_xl_cb	)\
/*	LD_R_YP		*/	X(LOG_OPCODE_LD_R_YP,	0xEB0,	MASK_10B,	0, 			0    , 		5 , 	&op_ld_r_yp_cb	)\
/*	LD_R_YH		*/	X(LOG_OPCODE_LD_R_YH,	0xEB4,	MASK_10B,	0, 			0    , 		5 , 	&op_ld_r_yh_cb	)\
/*	LD_R_YL		*/	X(LOG_OPCODE_LD_R_YL,	0xEB8,	MASK_10B,	0, 			0    , 		5 , 	&op_ld_r_yl_cb	)\
/*	ADC_XH		*/	X(LOG_OPCODE_ADC_XH,	0xA00,	MASK_8B,	0, 			0    , 		7 , 	&op_adc_xh_cb	)\
/*	ADC_XL		*/	X(LOG_OPCODE_ADC_XL,	0xA10,	MASK_8B,	0, 			0    , 		7 , 	&op_adc_xl_cb	)\
/*	ADC_YH		*/	X(LOG_OPCODE_ADC_YH,	0xA20,	MASK_8B,	0, 			0    , 		7 , 	&op_adc_yh_cb	)\
/*	ADC_YL		*/	X(LOG_OPCODE_ADC_YL,	0xA30,	MASK_8B,	0, 			0    , 		7 , 	&op_adc_yl_cb	)\
/*	CP_XH		*/	X(LOG_OPCODE_CP_XH,		0xA40,	MASK_8B,	0, 			0    , 		7 , 	&op_cp_xh_cb	)\
/*	CP_XL		*/	X(LOG_OPCODE_CP_XL,		0xA50,	MASK_8B,	0, 			0    , 		7 , 	&op_cp_xl_cb	)\
/*	CP_YH		*/	X(LOG_OPCODE_CP_YH,		0xA60,	MASK_8B,	0, 			0    , 		7 , 	&op_cp_yh_cb	)\
/*	CP_YL		*/	X(LOG_OPCODE_CP_YL,		0xA70,	MASK_8B,	0, 			0    , 		7 , 	&op_cp_yl_cb	)\
/*	LD_R_I		*/	X(LOG_OPCODE_LD_R_I,	0xE00,	MASK_6B,	4, 			0x030, 		5 , 	&op_ld_r_i_cb	)\
/*	LD_R_Q		*/	X(LOG_OPCODE_LD_R_Q,	0xEC0,	MASK_8B,	2, 			0x00C, 		5 , 	&op_ld_r_q_cb	)\
/*	LD_A_MN		*/	X(LOG_OPCODE_LD_A_MN,	0xFA0,	MASK_8B,	0, 			0    , 		5 , 	&op_ld_a_mn_cb	)\
/*	LD_B_MN		*/	X(LOG_OPCODE_LD_B_MN,	0xFB0,	MASK_8B,	0, 			0    , 		5 , 	&op_ld_b_mn_cb	)\
/*	LD_MN_A		*/	X(LOG_OPCODE_LD_MN_A,	0xF80,	MASK_8B,	0, 			0    , 		5 , 	&op_ld_mn_a_cb	)\
/*	LD_MN_B		*/	X(LOG_OPCODE_LD_MN_B,	0xF90,	MASK_8B,	0, 			0    , 		5 , 	&op_ld_mn_b_cb	)\
/*	LDPX_MX		*/	X(LOG_OPCODE_LDPX_MX,	0xE60,	MASK_8B,	0, 			0    , 		5 , 	&op_ldpx_mx_cb	)\
/*	LDPX_R		*/	X(LOG_OPCODE_LDPX_R,	0xEE0,	MASK_8B,	2, 			0x00C, 		5 , 	&op_ldpx_r_cb	)\
/*	LDPY_MY		*/	X(LOG_OPCODE_LDPY_MY,	0xE70,	MASK_8B,	0, 			0    , 		5 , 	&op_ldpy_my_cb	)\
/*	LDPY_R		*/	X(LOG_OPCODE_LDPY_R,	0xEF0,	MASK_8B,	2, 			0x00C, 		5 , 	&op_ldpy_r_cb	)\
/*	LBPX		*/	X(LOG_OPCODE_LBPX,		0x900,	MASK_4B,	0, 			0    , 		5 , 	&op_lbpx_cb		)\
/*	SET			*/	X(LOG_OPCODE_SET,		0xF40,	MASK_8B,	0, 			0    , 		7 , 	&op_set_cb		)\
/*	RST			*/	X(LOG_OPCODE_RST,		0xF50,	MASK_8B,	0, 			0    , 		7 , 	&op_rst_cb		)\
/*	SCF			*/	X(LOG_OPCODE_SCF,		0xF41,	MASK_12B,	0, 			0    , 		7 , 	&op_scf_cb		)\
/*	RCF			*/	X(LOG_OPCODE_RCF,		0xF5E,	MASK_12B,	0, 			0    , 		7 , 	&op_rcf_cb		)\
/*	SZF			*/	X(LOG_OPCODE_SZF,		0xF42,	MASK_12B,	0, 			0    , 		7 , 	&op_szf_cb		)\
/*	RZF			*/	X(LOG_OPCODE_RZF,		0xF5D,	MASK_12B,	0, 			0    , 		7 , 	&op_rzf_cb		)\
/*	SDF			*/	X(LOG_OPCODE_SDF,		0xF44,	MASK_12B,	0, 			0    , 		7 , 	&op_sdf_cb		)\
/*	RDF			*/	X(LOG_OPCODE_RDF,		0xF5B,	MASK_12B,	0, 			0    , 		7 , 	&op_rdf_cb		)\
/*	EI			*/	X(LOG_OPCODE_EI,		0xF48,	MASK_12B,	0, 			0    , 		7 , 	&op_ei_cb		)\
/*	DI			*/	X(LOG_OPCODE_DI,		0xF57,	MASK_12B,	0, 			0    , 		7 , 	&op_di_cb		)\
/*	INC_SP		*/	X(LOG_OPCODE_INC_SP,	0xFDB,	MASK_12B,	0, 			0    , 		5 , 	&op_inc_sp_cb	)\
/*	DEC_SP		*/	X(LOG_OPCODE_DEC_SP,	0xFCB,	MASK_12B,	0, 			0    , 		5 , 	&op_dec_sp_cb	)\
/*	PUSH_R		*/	X(LOG_OPCODE_PUSH_R,	0xFC0,	MASK_10B,	0, 			0    , 		5 , 	&op_push_r_cb	)\
/*	PUSH_XP		*/	X(LOG_OPCODE_PUSH_XP,	0xFC4,	MASK_12B,	0, 			0    , 		5 , 	&op_push_xp_cb	)\
/*	PUSH_XH		*/	X(LOG_OPCODE_PUSH_XH,	0xFC5,	MASK_12B,	0, 			0    , 		5 , 	&op_push_xh_cb	)\
/*	PUSH_XL		*/	X(LOG_OPCODE_PUSH_XL,	0xFC6,	MASK_12B,	0, 			0    , 		5 , 	&op_push_xl_cb	)\
/*	PUSH_YP		*/	X(LOG_OPCODE_PUSH_YP,	0xFC7,	MASK_12B,	0, 			0    , 		5 , 	&op_push_yp_cb	)\
/*	PUSH_YH		*/	X(LOG_OPCODE_PUSH_YH,	0xFC8,	MASK_12B,	0, 			0    , 		5 , 	&op_push_yh_cb	)\
/*	PUSH_YL		*/	X(LOG_OPCODE_PUSH_YL,	0xFC9,	MASK_12B,	0, 			0    , 		5 , 	&op_push_yl_cb	)\
/*	PUSH_F		*/	X(LOG_OPCODE_PUSH_F,	0xFCA,	MASK_12B,	0, 			0    , 		5 , 	&op_push_f_cb	)\
/*	POP_R		*/	X(LOG_OPCODE_POP_R,		0xFD0,	MASK_10B,	0, 			0    , 		5 , 	&op_pop_r_cb	)\
/*	POP_XP		*/	X(LOG_OPCODE_POP_XP,	0xFD4,	MASK_12B,	0, 			0    , 		5 , 	&op_pop_xp_cb	)\
/*	POP_XH		*/	X(LOG_OPCODE_POP_XH,	0xFD5,	MASK_12B,	0, 			0    , 		5 , 	&op_pop_xh_cb	)\
/*	POP_XL		*/	X(LOG_OPCODE_POP_XL,	0xFD6,	MASK_12B,	0, 			0    , 		5 , 	&op_pop_xl_cb	)\
/*	POP_YP		*/	X(LOG_OPCODE_POP_YP,	0xFD7,	MASK_12B,	0, 			0    , 		5 , 	&op_pop_yp_cb	)\
/*	POP_YH		*/	X(LOG_OPCODE_POP_YH,	0xFD8,	MASK_12B,	0, 			0    , 		5 , 	&op_pop_yh_cb	)\
/*	POP_YL		*/	X(LOG_OPCODE_POP_YL,	0xFD9,	MASK_12B,	0, 			0    , 		5 , 	&op_pop_yl_cb	)\
/*	POP_F		*/	X(LOG_OPCODE_POP_F,		0xFDA,	MASK_12B,	0, 			0    , 		5 , 	&op_pop_f_cb	)\
/*	LD_SPH_R	*/	X(LOG_OPCODE_LD_SPH_R,	0xFE0,	MASK_10B,	0, 			0    , 		5 , 	&op_ld_sph_r_cb	)\
/*	LD_SPL_R	*/	X(LOG_OPCODE_LD_SPL_R,	0xFF0,	MASK_10B,	0, 			0    , 		5 , 	&op_ld_spl_r_cb	)\
/*	LD_R_SPH	*/	X(LOG_OPCODE_LD_R_SPH,	0xFE4,	MASK_10B,	0, 			0    , 		5 , 	&op_ld_r_sph_cb	)\
/*	LD_R_SPL	*/	X(LOG_OPCODE_LD_R_SPL,	0xFF4,	MASK_10B,	0, 			0    , 		5 , 	&op_ld_r_spl_cb	)\
/*	ADD_R_I		*/	X(LOG_OPCODE_ADD_R_I,	0xC00,	MASK_6B,	4, 			0x030, 		7 , 	&op_add_r_i_cb	)\
/*	ADD_R_Q		*/	X(LOG_OPCODE_ADD_R_Q,	0xA80,	MASK_8B,	2, 			0x00C, 		7 , 	&op_add_r_q_cb	)\
/*	ADC_R_I		*/	X(LOG_OPCODE_ADC_R_I,	0xC40,	MASK_6B,	4, 			0x030, 		7 , 	&op_adc_r_i_cb	)\
/*	ADC_R_Q		*/	X(LOG_OPCODE_ADC_R_Q,	0xA90,	MASK_8B,	2, 			0x00C, 		7 , 	&op_adc_r_q_cb	)\
/*	SUB			*/	X(LOG_OPCODE_SUB,		0xAA0,	MASK_8B,	2, 			0x00C, 		7 , 	&op_sub_cb		)\
/*	SBC_R_I		*/	X(LOG_OPCODE_SBC_R_I,	0xD40,	MASK_6B,	4, 			0x030, 		7 , 	&op_sbc_r_i_cb	)\
/*	SBC_R_Q		*/	X(LOG_OPCODE_SBC_R_Q,	0xAB0,	MASK_8B,	2, 			0x00C, 		7 , 	&op_sbc_r_q_cb	)\
/*	AND_R_I		*/	X(LOG_OPCODE_AND_R_I,	0xC80,	MASK_6B,	4, 			0x030, 		7 , 	&op_and_r_i_cb	)\
/*	AND_R_Q		*/	X(LOG_OPCODE_AND_R_Q,	0xAC0,	MASK_8B,	2, 			0x00C, 		7 , 	&op_and_r_q_cb	)\
/*	OR_R_I		*/	X(LOG_OPCODE_OR_R_I,	0xCC0,	MASK_6B,	4, 			0x030, 		7 , 	&op_or_r_i_cb	)\
/*	OR_R_Q		*/	X(LOG_OPCODE_OR_R_Q,	0xAD0,	MASK_8B,	2, 			0x00C, 		7 , 	&op_or_r_q_cb	)\
/*	XOR_R_I		*/	X(LOG_OPCODE_XOR_R_I,	0xD00,	MASK_6B,	4, 			0x030, 		7 , 	&op_xor_r_i_cb	)\
/*	XOR_R_Q		*/	X(LOG_OPCODE_XOR_R_Q,	0xAE0,	MASK_8B,	2, 			0x00C, 		7 , 	&op_xor_r_q_cb	)\
/*	CP_R_I		*/	X(LOG_OPCODE_CP_R_I,	0xDC0,	MASK_6B,	4, 			0x030, 		7 , 	&op_cp_r_i_cb	)\
/*	CP_R_Q		*/	X(LOG_OPCODE_CP_R_Q,	0xF00,	MASK_8B,	2, 			0x00C, 		7 , 	&op_cp_r_q_cb	)\
/*	FAN_R_I		*/	X(LOG_OPCODE_FAN_R_I,	0xD80,	MASK_6B,	4, 			0x030, 		7 , 	&op_fan_r_i_cb	)\
/*	FAN_R_Q		*/	X(LOG_OPCODE_FAN_R_Q,	0xF10,	MASK_8B,	2, 			0x00C, 		7 , 	&op_fan_r_q_cb	)\
/*	RLC			*/	X(LOG_OPCODE_RLC,		0xAF0,	MASK_8B,	0, 			0    , 		7 , 	&op_rlc_cb		)\
/*	RRC			*/	X(LOG_OPCODE_RRC,		0xE8C,	MASK_10B,	0, 			0    , 		5 , 	&op_rrc_cb		)\
/*	INC_MN		*/	X(LOG_OPCODE_INC_MN,	0xF60,	MASK_8B,	0, 			0    , 		7 , 	&op_inc_mn_cb	)\
/*	DEC_MN		*/	X(LOG_OPCODE_DEC_MN,	0xF70,	MASK_8B,	0, 			0    , 		7 , 	&op_dec_mn_cb	)\
/*	ACPX		*/	X(LOG_OPCODE_ACPX,		0xF28,	MASK_10B,	0, 			0    , 		7 , 	&op_acpx_cb		)\
/*	ACPY		*/	X(LOG_OPCODE_ACPY,		0xF2C,	MASK_10B,	0, 			0    , 		7 , 	&op_acpy_cb		)\
/*	SCPX		*/	X(LOG_OPCODE_SCPX,		0xF38,	MASK_10B,	0, 			0    , 		7 , 	&op_scpx_cb		)\
/*	SCPY		*/	X(LOG_OPCODE_SCPY,		0xF3C,	MASK_10B,	0, 			0    , 		7 , 	&op_scpy_cb		)\
/*	NOT			*/	X(LOG_OPCODE_NOT,		0xD0F,	0xFCF,		4, 			0    , 		7 , 	&op_not_cb		)


#endif