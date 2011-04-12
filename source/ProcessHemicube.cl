
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

typedef unsigned int uint32_t; // opencl nema uint32_t

__kernel void ProcessHemicube(
		__global uint32_t *p_ids,				// patche, kterym nalezi energie, indexovano hodnotami p_write_index
		__global float *p_energies,				// soucty energii, indexovano hodnotami p_write_index
		__global unsigned int *p_write_index,	// index k zapisu do p_ids a p_energies, sdileny mezi instancemi, atomicky posouvany
		__read_only image2d_t t_patchview,		// textura ve ktere je pohled z nejakeho patche
		__read_only sampler_t n_sampler,		// sampler textury patchview
		__global const float *p_ffactors,		// textura form factoru
		const unsigned int n_width,				// sirka textury
		const unsigned int n_height,			// vyska textury
		const unsigned int n_span_length)		// delka jedne scanline, kterou zpracovava jedna instance
{
   int x0 = min(get_global_id(0) * n_span_length, n_width - 1); // od
   int x1 = min(x0 + n_span_length, n_width);					// do
   int y = get_global_id(1);
   // spocitaji se souradnice vypoctu jednoho vlakna

   if(y >= n_height)
       return;
   // vlakna za obrazem

   //p_patchview += n_width * y;
   p_ffactors += n_width * y; // posun na aktualni scanline

   while(x0 < x1) {
	   float4 color = read_imagef(t_patchview, n_sampler, (int2)(x0, y));
	   uint32_t n_patch_id = 1048576 * (uint32_t)(color.z * 1024) + 1024 * (uint32_t)(color.y * 1024) + (uint32_t)(color.x * 1024);

		float f_energy = 0;
		while (x0 < x1) {
			//for(; x0 < x1 && p_patchview[x0] == n_patch_id; ++ x0)
			float4 act_color = read_imagef(t_patchview, n_sampler, (int2)(x0, y));
			uint32_t n_act_patch = 1048576 * (uint32_t)(act_color.z * 1024) + 1024 * (uint32_t)(act_color.y * 1024) + (uint32_t)(act_color.x * 1024);
			if (n_act_patch == n_patch_id) {
				f_energy += p_ffactors[x0]; // suma energie pro jeden polygon na jedne scanline
				x0++;
			} else
				break;
		}

	   if (n_patch_id > 0) { // cerne patche nebrat - zrejme nepresne uzavreny prostor
		unsigned int n_write_id = atomic_inc(p_write_index);
		p_ids[n_write_id] = unpack(n_patch_id + correction) - 1;
		p_energies[n_write_id] = f_energy;
		// zapsat energii do globalniho pole
	   }
   }

}
