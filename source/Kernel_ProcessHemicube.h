

const char* kernel_processHemicube = 	
		"#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable\n"
		"typedef unsigned int uint32_t;\n" // opencl nema uint32_t

		"__kernel void ProcessHemicube(\n"
		"		__global uint32_t *p_hemicubes,\n"			// cislo pohledu, ke kteremu se vztahuji p_ids a p_energies; indexovano p_write_index
		"		__global uint32_t *p_ids,\n"				// patche, kterym nalezi energie, indexovano hodnotami p_write_index
		"		__global float *p_energies,\n"				// soucty energii, indexovano hodnotami p_write_index
		"		__global unsigned int *p_write_index,\n"	// index k zapisu do p_ids a p_energies, sdileny mezi instancemi, atomicky posouvany
		"		__read_only image2d_t t_patchview,\n"		// textura ve ktere je pohled z nejakeho patche
		"		__read_only sampler_t n_sampler,\n"			// sampler textury patchview
		"		__global const float *p_ffactors,\n"		// textura form factoru
		"		const unsigned int n_width,\n"				// sirka textury
		"		const unsigned int n_height,\n"				// vyska textury
		"		const unsigned int n_span_length,\n"		// delka jedne scanline, kterou zpracovava jedna instance
		"		const unsigned int n_hemicubes)\n"			// pocet hemicubes; celkova vyska textury = n_hemicubes * n_height
		"{\n"
			// spocitaji se souradnice vypoctu jednoho vlakna
		"	int x0 = min(get_global_id(0) * n_span_length, n_width - 1);\n"		// od
		"	int x1 = min(x0 + n_span_length, n_width);\n"						// do
		"	int y = get_global_id(1);\n"
		   
			// vlakna za obrazem
		"	if(y >= n_height * n_hemicubes)\n"
		"		return;\n"
		   
			// posun na aktualni scanline
		"	p_ffactors += n_width * y;\n" 

		"	while(x0 < x1) {\n"
		"	   float4 color = read_imagef(t_patchview, n_sampler, (int2)(x0, y));\n"
		"	   uint32_t n_patch_id = 1048576 * (uint32_t)(color.z * 1024) + 1024 * (uint32_t)(color.y * 1024) + (uint32_t)(color.x * 1024);\n"

		"		float f_energy = 0;\n"
		"		while (x0 < x1) {\n"
		"			float4 act_color = read_imagef(t_patchview, n_sampler, (int2)(x0, y));\n"
		"			uint32_t n_act_patch = 1048576 * (uint32_t)(act_color.z * 1024) + 1024 * (uint32_t)(act_color.y * 1024) + (uint32_t)(act_color.x * 1024);\n"
		"			if (n_act_patch == n_patch_id) {\n"
		"				if (p_ffactors[x0]==0)\n"
		"					f_energy = 999;"
		"				else\n"
		"					f_energy += p_ffactors[x0];\n" // suma energie pro jeden polygon na jedne scanline
		"				x0++;\n"
		"			} else\n"
		"				break;\n"
		"		}\n"

		"	   if (n_patch_id > 0) {\n"	// cerne patche nebrat - zrejme nepresne uzavreny prostor
		"		unsigned int n_write_id = atomic_inc(p_write_index);\n"
		"		p_hemicubes[n_write_id] = y / n_height;\n"
		"		p_ids[n_write_id] = unpack(n_patch_id + correction) - 1;\n"
		"		p_energies[n_write_id] = f_energy;\n"	// zapsat energii do globalniho pole				
		"	   }\n"
		"	}\n"
		"}\n";
