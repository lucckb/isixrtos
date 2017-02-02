/*
 * =====================================================================================
 *
 *       Filename:  test_energy_meter.cpp
 *
 *    Description:  Energy meter class test
 *
 *        Version:  1.0
 *        Created:  10.10.2016 21:13:04
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <cmath>
#include <emeter/energy_meter.hpp>
#include <gtest/gtest.h>
#include <boost/lexical_cast.hpp>
#include "simulate_pwr_ui.hpp"
#include "gprint.hpp"



//Helper function for apply buffer to all UI data
namespace {
	constexpr auto bufsiz = emeter::config::fftbuf_size;
	namespace tg = emeter::tags;
	constexpr auto u_scale =  333.333 * 1.65;
	constexpr auto i_scale = 1.65 * 5.0;
	constexpr auto ctr_val = 20.0;

	// Apply energy meter on time
	void process_time( emeter::energy_meter& em, const sim::phases_buf_t& U,
			const sim::phases_buf_t& I )
	{
		for( std::size_t s=0; s<I[0].size(); s+= bufsiz ) {
			for( std::size_t ph=0; ph<I.size(); ++ph ) {
				//Current processing
				auto oh = em.sample_current_begin(ph);
				if(!oh) {
					throw std::logic_error("SCB");
				}
				std::copy( &I[ph][s], &I[ph][s+bufsiz], oh );
				if( em.sample_current_end( ph ) ) {
					throw std::logic_error("SCE");
				}
				//Voltage processing
				auto vb = em.sample_voltage_begin(ph);
				if(!vb) {
					throw std::logic_error("SCB");
				}
				std::copy( &U[ph][s], &U[ph][s+bufsiz], vb );
				if( em.sample_voltage_end( ph ) ) {
					throw std::logic_error("SCE");
				}
			}
			if( em.calculate() ) {
				throw std::logic_error("CALC");
			}
		}
	}
	void dump_energies( const emeter::energy_meter& em )
	{
		PRINTF("1: P %f Q %f\n", em(0,tg::p_avg), em(0,tg::q_avg) );
		PRINTF("2: P %f Q %f\n", em(1,tg::p_avg), em(1,tg::q_avg) );
		PRINTF("3: P %f Q %f\n", em(2,tg::p_avg), em(2,tg::q_avg) );
		PRINTF("1: U %f I %f\n", em(0,tg::u_rms), em(0, tg::i_rms) );
		PRINTF("2: U %f I %f\n", em(1,tg::u_rms), em(1, tg::i_rms) );
		PRINTF("3: U %f I %f\n", em(2,tg::u_rms), em(2, tg::i_rms) );
		PRINTF("1:E+ %f Q+ %f E- %f Q- %f\n", em(0,tg::watt_h_pos), em(0,tg::var_h_pos),
				em(0,tg::watt_h_neg), em(0,tg::var_h_neg ) );
		PRINTF("2:E+ %f Q+ %f E- %f Q- %f\n", em(1,tg::watt_h_pos), em(1,tg::var_h_pos),
				em(1,tg::watt_h_neg), em(1,tg::var_h_neg ) );
		PRINTF("3:E+ %f Q+ %f E- %f Q- %f\n", em(2,tg::watt_h_pos), em(2,tg::var_h_pos),
				em(2,tg::watt_h_neg), em(2,tg::var_h_neg ) );
		PRINTF("3f: E+ %f Q+ %f E- %f Q- %f\n", em(tg::watt_h_pos), em(tg::var_h_pos),
				em(tg::watt_h_neg), em(tg::var_h_neg ) );
	}
}

// Base rective energy only caculate up to 1KW
TEST( energy_meter, long_r_only ) {
	emeter::energy_meter em;
	em.set_scale_u( u_scale );
	em.set_scale_i( i_scale );
	em.set_ctr_ratio( ctr_val );
	constexpr auto sim_duration = 3600*10;
	constexpr auto voltage = 230.0;
	constexpr auto current = 1.0;
	constexpr auto freq = 50;
	constexpr auto angle = 0;
	auto v_vect = sim::generate_sinus( voltage, freq, angle, sim_duration );
	auto i_vect = sim::generate_sinus( current, freq, 0, sim_duration, 1 );
	process_time( em, v_vect, i_vect );
	for( std::size_t ph=0; ph<3; ++ph ) {
		SCOPED_TRACE( "PHASE("+ boost::lexical_cast<std::string>(ph)+ ")" );
		EXPECT_NEAR( em(ph,tg::p_avg), 23000 , 0.4 );
		EXPECT_NEAR( em(ph,tg::q_avg), 0 , 0.15 );
		EXPECT_NEAR( em(ph,tg::u_rms), 230 , 0.1 );
		EXPECT_NEAR( em(ph,tg::i_rms), 100 , 0.1 );
		EXPECT_NEAR( em(ph,tg::watt_h_pos), 230000 , 0.8 );
		EXPECT_EQ( em(ph,tg::watt_h_neg), 0 );
		EXPECT_EQ( em(ph,tg::var_h_pos), 0 );
		EXPECT_EQ( em(ph,tg::var_h_neg), 0 );
	}
	EXPECT_NEAR( em(tg::watt_h_pos), 230000*3 , 0.8*3 );
	EXPECT_EQ( em(tg::watt_h_neg), 0 );
	EXPECT_EQ( em(tg::var_h_pos), 0 );
	EXPECT_EQ( em(tg::var_h_neg), 0 );
	dump_energies( em );

}


// Base rective energy only caculate up to 1KW
TEST( energy_meter, short_r_only ) {
	emeter::energy_meter em;
	em.set_scale_u( u_scale );
	em.set_scale_i( i_scale );
	em.set_ctr_ratio( ctr_val );
	constexpr auto sim_duration = 3600*0.5;
	constexpr auto voltage = 230.0;
	constexpr auto current = 1.0;
	constexpr auto freq = 50;
	constexpr auto angle = 0;
	auto v_vect = sim::generate_sinus( voltage, freq, angle, sim_duration );
	auto i_vect = sim::generate_sinus( current, freq, 0, sim_duration, 1 );
	process_time( em, v_vect, i_vect );
	for( std::size_t ph=0; ph<3; ++ph ) {
		SCOPED_TRACE( "PHASE("+ boost::lexical_cast<std::string>(ph)+ ")" );
		EXPECT_NEAR( em(ph,tg::p_avg), 23000 , 0.4 );
		EXPECT_NEAR( em(ph,tg::q_avg), 0 , 0.12 );
		EXPECT_NEAR( em(ph,tg::u_rms), 230 , 0.1 );
		EXPECT_NEAR( em(ph,tg::i_rms), 100 , 0.1 );
		EXPECT_NEAR( em(ph,tg::watt_h_pos), 11500 , 0.1);
		EXPECT_EQ( em(ph,tg::watt_h_neg), 0 );
		EXPECT_EQ( em(ph,tg::var_h_pos), 0 );
		EXPECT_EQ( em(ph,tg::var_h_neg), 0 );
	}
	EXPECT_NEAR( em(tg::watt_h_pos), 11500*3 , 0.1);
	EXPECT_EQ( em(tg::watt_h_neg), 0 );
	EXPECT_EQ( em(tg::var_h_pos), 0 );
	EXPECT_EQ( em(tg::var_h_neg), 0 );
}

constexpr inline double p2err( double exc, double perc ) {
	return std::abs(exc) * perc * 0.01;
}

// Base rective energy only caculate up to 1KW
TEST( energy_meter, short_rl_motor_only ) {
	emeter::energy_meter em;
	em.set_scale_u( u_scale );
	em.set_scale_i( i_scale );
	em.set_ctr_ratio( ctr_val );
	constexpr auto sim_duration = 3600*0.5;
	constexpr auto voltage = 230.0;
	constexpr auto current = 1.0;
	constexpr auto freq = 50;
	constexpr auto angle = 45;
	constexpr auto Pexc = voltage * current * 100.0 * std::cos(sim::deg2rad(angle));
	constexpr auto Qexc = voltage * current * 100.0 * std::sin(sim::deg2rad(angle));
	auto v_vect = sim::generate_sinus( voltage, freq, angle, sim_duration );
	auto i_vect = sim::generate_sinus( current, freq, 0, sim_duration, 1 );
	process_time( em, v_vect, i_vect );
	for( std::size_t ph=0; ph<3; ++ph ) {
		SCOPED_TRACE( "PHASE("+ boost::lexical_cast<std::string>(ph)+ ")" );
		EXPECT_NEAR( em(ph,tg::p_avg), Pexc , 2 );
		EXPECT_NEAR( em(ph,tg::q_avg), Qexc , 2 );
		EXPECT_NEAR( em(ph,tg::u_rms), 230 , 0.1 );
		EXPECT_NEAR( em(ph,tg::i_rms), 100 , 0.1 );
		EXPECT_NEAR( em(ph,tg::watt_h_pos), (Pexc/2) , p2err((Pexc/2), 0.006) );
		EXPECT_EQ( em(ph,tg::watt_h_neg), 0 );
		EXPECT_NEAR( em(ph,tg::var_h_pos), (Qexc/2) , p2err((Qexc/2), 0.006) );
		EXPECT_EQ( em(ph,tg::var_h_neg), 0 );
	}
	EXPECT_NEAR( em(tg::watt_h_pos), (Pexc/2)*3.0 , p2err((Pexc/2)*3.0, 0.006 ) );
	EXPECT_EQ( em(tg::watt_h_neg), 0 );
	EXPECT_NEAR( em(tg::var_h_pos), (Qexc/2)*3.0, p2err((Qexc/2)*3.0, 0.006 )  );
	EXPECT_EQ( em(tg::var_h_neg), 0 );

}


// Base rective energy only caculate up to 1KW
TEST( energy_meter, short_rl_gen_qm ) {
	emeter::energy_meter em;
	em.set_scale_u( u_scale );
	em.set_scale_i( i_scale );
	em.set_ctr_ratio( ctr_val );
	constexpr auto sim_duration = 3600*0.5;
	constexpr auto voltage = 230.0;
	constexpr auto current = 1.0;
	constexpr auto freq = 50;
	constexpr auto angle = -45;
	constexpr auto Pexc = voltage * current * 100.0 * std::cos(sim::deg2rad(angle));
	constexpr auto Qexc = voltage * current * 100.0 * std::sin(sim::deg2rad(angle));
	auto v_vect = sim::generate_sinus( voltage, freq, angle, sim_duration );
	auto i_vect = sim::generate_sinus( current, freq, 0, sim_duration, 1 );
	process_time( em, v_vect, i_vect );
	for( std::size_t ph=0; ph<3; ++ph ) {
		SCOPED_TRACE( "PHASE("+ boost::lexical_cast<std::string>(ph)+ ")" );
		EXPECT_NEAR( em(ph,tg::p_avg), Pexc , 2 );
		EXPECT_NEAR( em(ph,tg::q_avg), Qexc , 2 );
		EXPECT_NEAR( em(ph,tg::u_rms), 230 , 0.1 );
		EXPECT_NEAR( em(ph,tg::i_rms), 100 , 0.1 );
		EXPECT_NEAR( em(ph,tg::watt_h_pos), (Pexc/2), p2err((Pexc/2), 0.006) );
		EXPECT_EQ( em(ph,tg::watt_h_neg), 0 );
		EXPECT_NEAR( em(ph,tg::var_h_neg), -(Qexc/2), p2err((Qexc/2), 0.006) );
		EXPECT_EQ( em(ph,tg::var_h_pos), 0 );
	}
	EXPECT_NEAR( em(tg::watt_h_pos), (Pexc/2)*3.0 , p2err((Pexc/2)*3.0, 0.006 ) );
	EXPECT_EQ( em(tg::watt_h_neg), 0 );
	EXPECT_NEAR( em(tg::var_h_neg), -(Qexc/2)*3.0, p2err((Qexc/2)*3.0, 0.006 )  );
	EXPECT_EQ( em(tg::var_h_pos), 0 );

}


// Base rective energy only caculate up to 1KW
TEST( energy_meter, short_rl_gen_qm_pm ) {
	emeter::energy_meter em;
	em.set_scale_u( u_scale );
	em.set_scale_i( i_scale );
	em.set_ctr_ratio( ctr_val );
	constexpr auto sim_duration = 3600*0.5;
	constexpr auto voltage = 230.0;
	constexpr auto current = 1.0;
	constexpr auto freq = 50;
	constexpr auto angle = -45*3;
	constexpr auto Pexc = voltage * current * 100.0 * std::cos(sim::deg2rad(angle));
	constexpr auto Qexc = voltage * current * 100.0 * std::sin(sim::deg2rad(angle));
	auto v_vect = sim::generate_sinus( voltage, freq, angle, sim_duration );
	auto i_vect = sim::generate_sinus( current, freq, 0, sim_duration, 1 );
	process_time( em, v_vect, i_vect );
	for( std::size_t ph=0; ph<3; ++ph ) {
		SCOPED_TRACE( "PHASE("+ boost::lexical_cast<std::string>(ph)+ ")" );
		EXPECT_NEAR( em(ph,tg::p_avg), Pexc , 2 );
		EXPECT_NEAR( em(ph,tg::q_avg), Qexc , 2 );
		EXPECT_NEAR( em(ph,tg::u_rms), 230 , 0.1 );
		EXPECT_NEAR( em(ph,tg::i_rms), 100 , 0.1 );
		EXPECT_NEAR( em(ph,tg::watt_h_neg), -(Pexc/2), p2err((Pexc/2), 0.006) );
		EXPECT_EQ( em(ph,tg::watt_h_pos), 0 );
		EXPECT_NEAR( em(ph,tg::var_h_neg), -(Qexc/2), p2err((Qexc/2), 0.006) );
		EXPECT_EQ( em(ph,tg::var_h_pos), 0 );
	}
	EXPECT_NEAR( em(tg::watt_h_neg), -(Pexc/2)*3.0 , p2err((Pexc/2)*3.0, 0.006 ) );
	EXPECT_EQ( em(tg::watt_h_pos), 0 );
	EXPECT_NEAR( em(tg::var_h_neg), -(Qexc/2)*3.0, p2err((Qexc/2)*3.0, 0.006 )  );
	EXPECT_EQ( em(tg::var_h_pos), 0 );

}




// Base rective energy only caculate up to 1KW
TEST( energy_meter, short_r_trigger_current_bellow ) {
	emeter::energy_meter em;
	em.set_scale_u( u_scale );
	em.set_scale_i( i_scale );
	em.set_ctr_ratio( ctr_val );
	constexpr auto sim_duration = 3600*0.5;
	constexpr auto voltage = 230.0;
	constexpr auto current = 0.09;
	constexpr auto freq = 50;
	constexpr auto angle = 0;
	constexpr auto Pexc = voltage * current * std::cos(sim::deg2rad(angle));
	constexpr auto Qexc = voltage * current * std::sin(sim::deg2rad(angle));
	auto v_vect = sim::generate_sinus( voltage, freq, angle, sim_duration );
	auto i_vect = sim::generate_sinus( current/100.0, freq, 0, sim_duration, 1 );
	process_time( em, v_vect, i_vect );
	for( std::size_t ph=0; ph<3; ++ph ) {
		SCOPED_TRACE( "PHASE("+ boost::lexical_cast<std::string>(ph)+ ")" );
		EXPECT_NEAR( em(ph,tg::p_avg), Pexc , 0.2 );
		EXPECT_NEAR( em(ph,tg::q_avg), Qexc , 0.2 );
		EXPECT_NEAR( em(ph,tg::u_rms), 230 , 0.1 );
		EXPECT_NEAR( em(ph,tg::i_rms), current , 0.01 );
		EXPECT_EQ( em(ph,tg::watt_h_neg), 0 );
		EXPECT_EQ( em(ph,tg::watt_h_pos), 0 );
		EXPECT_EQ( em(ph,tg::var_h_neg), 0  );
		EXPECT_EQ( em(ph,tg::var_h_pos), 0 );
	}
	EXPECT_EQ( em(tg::watt_h_neg), 0 );
	EXPECT_EQ( em(tg::watt_h_pos), 0 );
	EXPECT_EQ( em(tg::var_h_pos), 0 );
	EXPECT_EQ( em(tg::var_h_neg), 0 );

}





// Base rective energy only caculate up to 1KW
TEST( energy_meter, short_r_trigger_current_over ) {
	emeter::energy_meter em;
	em.set_scale_u( u_scale );
	em.set_scale_i( i_scale );
	em.set_ctr_ratio( ctr_val );
	constexpr auto sim_duration = 3600*0.5;
	constexpr auto voltage = 230.0;
	constexpr auto current = 0.11;
	constexpr auto freq = 50;
	constexpr auto angle = 0;
	constexpr auto Pexc = voltage * current * std::cos(sim::deg2rad(angle));
	auto v_vect = sim::generate_sinus( voltage, freq, angle, sim_duration );
	auto i_vect = sim::generate_sinus( current/100.0, freq, 0, sim_duration, 1 );
	process_time( em, v_vect, i_vect );
	for( std::size_t ph=0; ph<3; ++ph ) {
		SCOPED_TRACE( "PHASE("+ boost::lexical_cast<std::string>(ph)+ ")" );
		EXPECT_NEAR( em(ph,tg::p_avg), Pexc , 0.09 );
		EXPECT_NEAR( em(ph,tg::q_avg),  0, 0.2 );
		EXPECT_NEAR( em(ph,tg::u_rms), 230 , 0.1 );
		EXPECT_NEAR( em(ph,tg::i_rms), current , 0.01 );
		EXPECT_NEAR( em(ph,tg::watt_h_pos), Pexc/2.0, 0.09 );
		EXPECT_EQ( em(ph,tg::watt_h_neg), 0 );
		EXPECT_EQ( em(ph,tg::var_h_neg), 0  );
		EXPECT_EQ( em(ph,tg::var_h_pos), 0 );
	}
	EXPECT_NEAR( em(tg::watt_h_pos), (Pexc/2.0)*3.0, 0.09 );
	EXPECT_EQ( em(tg::watt_h_neg), 0 );
	EXPECT_EQ( em(tg::var_h_pos), 0 );
	EXPECT_EQ( em(tg::var_h_neg), 0 );
}


// Base rective energy only caculate up to 1KW
TEST( energy_meter, short_r_trigger_pow_under ) {
	emeter::energy_meter em;
	em.set_scale_u( u_scale );
	em.set_scale_i( i_scale );
	em.set_ctr_ratio( ctr_val );
	constexpr auto sim_duration = 3600*0.5;
	constexpr auto voltage = 230.0;
	constexpr auto current = 0.11;
	constexpr auto freq = 50;
	constexpr auto angle = 45;
	constexpr auto Pexc = voltage * current * std::cos(sim::deg2rad(angle));
	constexpr auto Qexc = voltage * current * std::sin(sim::deg2rad(angle));
	auto v_vect = sim::generate_sinus( voltage, freq, angle, sim_duration );
	auto i_vect = sim::generate_sinus( current/100.0, freq, 0, sim_duration, 1 );
	process_time( em, v_vect, i_vect );
	for( std::size_t ph=0; ph<3; ++ph ) {
		SCOPED_TRACE( "PHASE("+ boost::lexical_cast<std::string>(ph)+ ")" );
		EXPECT_NEAR( em(ph,tg::p_avg), Pexc , 0.16 );
		EXPECT_NEAR( em(ph,tg::q_avg), Qexc, 0.16 );
		EXPECT_NEAR( em(ph,tg::u_rms), 230 , 0.1 );
		EXPECT_NEAR( em(ph,tg::i_rms), current , 0.01 );
		EXPECT_NEAR( em(ph,tg::watt_h_pos), Pexc/2.0, 0.12 );
		EXPECT_EQ( em(ph,tg::watt_h_neg), 0 );
		EXPECT_EQ( em(ph,tg::var_h_neg), 0  );
		EXPECT_NEAR( em(ph,tg::var_h_pos), Qexc/2.0, 0.12);
	}
	EXPECT_NEAR( em(tg::watt_h_pos), (Pexc/2.0)*3.0, 0.12 );
	EXPECT_EQ( em(tg::watt_h_neg), 0 );
	EXPECT_NEAR( em(tg::var_h_pos), (Qexc/2.0)*3.0, 0.12 );
	EXPECT_EQ( em(tg::var_h_neg), 0 );
}




