
#include "../simconst.h"
#include "../simtypes.h"
#include "../simdebug.h"

#include "../simplay.h"
#include "../simfab.h"
#include "../simcity.h"
#include "../simworld.h"

#include "tabfile.h"
#include "loadsave.h"
#include "umgebung.h"

#include "scenario.h"


karte_t *scenario_t::welt = NULL;



scenario_t::scenario_t(karte_t *w)
{
	welt = w;
	what_scenario = 0;
	city = NULL;
	fab = NULL;
	scenario_name = NULL;
}



void scenario_t::init( const char *filename, karte_t *w )
{
	welt = w;
	what_scenario = 0;
	city = NULL;
	fab = NULL;
	if(scenario_name) {
		free( scenario_name );
	}
	scenario_name = NULL;

	tabfile_t scenario;

	char path[1024];
	sprintf( path, "%s.tab", filename );
	if (!scenario.open(filename)) {
		dbg->error("scenario_t::scenario_t()", "Can't read %s", path );
		return;
	}

	tabfileobj_t contents;
	scenario.read(contents);

	scenario_name = strdup( contents.get( "savegame" ) );
	what_scenario = contents.get_int( "type", 0 );

	// may have additional info like city ...
	const char *cityname = contents.get( "cityname" );
	city = NULL;
	if(*cityname) {
		// find a city with this name ...
		const weighted_vector_tpl<stadt_t*> staedte = welt->gib_staedte();
		for(  int i=0;  staedte.get_count();  i++  ) {
			if(  strcmp( staedte[i]->gib_name(), cityname )==0  ) {
				city = staedte[i];
			}
		}
	}

	// ... or factory
	int *pos = contents.get_ints( "factorypos" );
	fab = NULL;
	if(*pos==2) {
		fab = fabrik_t::gib_fab( welt, koord( pos[1], pos[2] ) );
	}
}



void scenario_t::rdwr(loadsave_t *file)
{
	int city_nr = 0x7FFFFFFFu;
	koord fabpos = koord::invalid;

	if(  file->is_saving()  ) {
		if(city) {
			city_nr = welt->gib_staedte().index_of( city );
		}
		if(  fab  ) {
			fabpos = fab->gib_pos().gib_2d();
		}
	}

	file->rdwr_short( what_scenario, "" );
	file->rdwr_long( city_nr, "" );
	fabpos.rdwr( file );

	if(  file->is_loading()  ) {
		if(  city_nr < welt->gib_staedte().get_count()  ) {
			city = welt->gib_staedte()[city_nr];
		}
		fab = fabrik_t::gib_fab( welt, fabpos );
	}
}



/* recursive lookup of a factory tree:
 * count ratio of needed versus producing factories
 */
void scenario_t::get_factory_producing( fabrik_t *fab, int &producing, int &existing )
{
	int own_producing=0, own_existing=0;

	// now check for all input
	for(  int ware_nr=0;  ware_nr<fab->gib_eingang().get_count();  ware_nr++  ) {
		if(fab->gib_eingang()[ware_nr].menge > 512) {
			producing ++;
			own_producing ++;
		}
		existing ++;
		own_existing ++;
	}

	if(fab->gib_eingang().get_count()>0) {
		// now check for all output (of not source ... )
		for(  int ware_nr=0;  ware_nr<fab->gib_ausgang().get_count();  ware_nr++  ) {
			if(fab->gib_ausgang()[ware_nr].menge > 512) {
				producing ++;
				own_producing ++;
			}
			existing ++;
			own_existing ++;
		}
	}

	// now all delivering factories
	const vector_tpl <koord> & sources = fab->get_suppliers();
	for( unsigned q=0;  q<sources.get_count();  q++  ) {
		fabrik_t *qfab = fabrik_t::gib_fab(welt,sources[q]);
		if(  own_producing==own_existing  ) {
			// fully supplied => counts as 100% ...
			int i=0, cnt=0;
			get_factory_producing( qfab, i, cnt );
			producing += cnt;
			existing += cnt;
		}
		else {
			// try something else ...
			get_factory_producing( qfab, producing, existing );
		}
	}
}



// return percentage completed
int scenario_t::completed(int player_nr)
{
	switch(  what_scenario  ) {

		case CONNECT_CITY_WORKER:
			// check, if there are connections to all factories from all over the town
			return 0;

		case CONNECT_FACTORY_PAX:
			// check, if there is complete coverage of all connected towns to this factory
			return 0;

		case CONNECT_FACTORY_GOODS:
		{
			// true, if this factory can produce (i.e. more than one unit per good in each input)
			int prod=0, avail=0;
			get_factory_producing( fab, prod, avail );
			return (prod*100)/avail;
		}

		case DOUBLE_INCOME:
		{
			int pts = (int)( ((welt->gib_spieler(player_nr)->get_finance_history_month(0,COST_CASH)-umgebung_t::starting_money)*100)/umgebung_t::starting_money );
			return min( 100, pts );
		}

		case BUILT_HEADQUARTER_AND_10_TRAINS:
		{
			spieler_t *sp = welt->gib_spieler(player_nr);
			int pts = sp->get_headquarter_pos() != koord::invalid ? 11 : 0;
			for (vector_tpl<convoihandle_t>::const_iterator i = welt->convois_begin(), end = welt->convois_end(); i != end; ++i) {
				convoihandle_t cnv = *i;
				if(  cnv->gib_besitzer() == sp  &&  cnv->gib_jahresgewinn()>0  ) {
					if(  pts<100  ) {
						pts += 9;
					}
				}
			}
			return pts;
		}

		case TRANSPORT_1000_PAX:
			return min( 100, welt->gib_spieler(player_nr)->get_finance_history_month(0,COST_TRANSPORTED_PAS)/10 );

	}
	return 0;
}