#include <string>
#include "../../dataobj/tabfile.h"
#include "../fussgaenger_besch.h"
#include "obj_node.h"
#include "text_writer.h"
#include "imagelist_writer.h"
#include "pedestrian_writer.h"


void pedestrian_writer_t::write_obj(FILE* fp, obj_node_t& parent, tabfileobj_t& obj)
{
	fussgaenger_besch_t besch;
	int i;

	obj_node_t node(this, 4, &parent);

	write_head(fp, node, obj);

	besch.gewichtung = obj.get_int("distributionweight", 1);

	static const char* const dir_codes[] = {
		"s", "w", "sw", "se", "n", "e", "ne", "nw"
	};
	slist_tpl<std::string> keys;
	std::string str;

	for (i = 0; i < 8; i++) {
		char buf[40];

		sprintf(buf, "image[%s]", dir_codes[i]);
		str = obj.get(buf);
		keys.append(str);
	}
	imagelist_writer_t::instance()->write_obj(fp, node, keys);

	node.write_uint16(fp, besch.gewichtung, 0);
	node.write_uint16(fp, 0,                2); //dummy, unused (and uninitialized in past versions)

	node.write(fp);
}
