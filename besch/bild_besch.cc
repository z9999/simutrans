#include "bild_besch.h"
#include "../simgraph.h"
#include "../simtypes.h"
#include "../simdebug.h"

#include <string.h>


typedef uint16 PIXVAL;

/* rotate_image_data - produces a (rotated) bild_besch
 * only rotates by 90 degrees or multiples thereof, and assumes a square image
 * Otherwise it will only succeed for angle=0;
*/
bild_besch_t *bild_besch_t::copy_rotate(const sint16 angle) const
{
	assert(angle == 0 || (pic.w == pic.h && pic.x == 0 && pic.y == 0));

	bild_besch_t* target_besch = new(pic.len * sizeof(PIXVAL)) bild_besch_t();
	target_besch->pic = pic;
	target_besch->pic.bild_nr = IMG_LEER;
	memcpy(target_besch->pic.data, pic.data, pic.len * sizeof(PIXVAL));

	// the format is
	// transparent PIXELVAL number
	// PIXEL number of pixels, data*PIXVAL
	// repeated until zero transparent pixels
	// in pak64 case it is 0 64 64*PIXVAL 0 for a single line, e.g. 70 bytes per line for pak64
	// first data will have an offset of two PIXVAL
	// now you should understand below arithmetics ...

	sint16        const x_y    = pic.w;
	PIXVAL const* const src    = get_daten();
	PIXVAL*       const target = target_besch->get_daten();

	switch(angle) {
		case 90:
			for(int j=0; j<x_y; j++) {
				for(int i=0; i<x_y; i++) {
					target[j*(x_y+3)+i+2]=src[i*(x_y+3)+(x_y-j-1)+2];
				}
			}
		break;

		case 180:
			for(int j=0; j<x_y; j++) {
				for(int i=0; i<x_y; i++) {
					target[j*(x_y+3)+i+2]=src[(x_y-j-1)*(x_y+3)+(x_y-i-1)+2];
				}
			}
		break;
		case 270:
			for(int j=0; j<x_y; j++) {
				for(int i=0; i<x_y; i++) {
					target[j*(x_y+3)+i+2]=src[(x_y-i-1)*(x_y+3)+j+2];
				}
			}
		break;
		default: // no rotation, just converts to array
			;
	}
	return target_besch;
}
