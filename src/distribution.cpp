#include <iostream>
#include "distribution.h"

namespace stenosys
{

static const uint32_t BAR_WIDTH          = 126;
static const uint32_t BUCKET_DISPLAY_MAX = 32;

#define COLUMN_MAX( x ) ( std::min( x, BUCKET_DISPLAY_MAX ) )


C_distribution::C_distribution( const char * title, uint32_t max_bucket, uint64_t bucket_width )
    : max_bucket_( max_bucket )
    , max_used_bucket_( 0 )
    , bucket_width_( bucket_width )
    , title_( title )
{
    buckets_ = new uint32_t[ max_bucket_ + 2 ];   // +1 for the 'greater than max' bucket

    for ( uint32_t ii = 0; ii <= max_bucket_ + 1; ii++ )
    {
        buckets_[ ii ] = 0;
    }
}

C_distribution::~C_distribution()
{
    delete [] buckets_;
}

void
C_distribution::add( uint64_t value )
{
    uint32_t bucket = ( uint32_t ) ( value / bucket_width_ );

    if ( bucket <= max_bucket_ )
    {
         buckets_[ bucket ]++;
    }
    else
    {
         buckets_[ max_bucket_ + 1 ]++;
    }

    if ( bucket > max_used_bucket_ )
    {
        max_used_bucket_ = bucket;
    }
}

std::string
C_distribution::report()
{
    uint32_t total_score = 0;   // Total of all counts

    for ( uint32_t bucket = 0; bucket <= COLUMN_MAX( max_bucket_ ); bucket++ )
    {
        total_score += buckets_[ bucket ];
    }
    
    std::string report;

    char buffer[ 400 ];
    
    report += "C: ";

    for ( uint32_t bucket = 0; bucket <= COLUMN_MAX( max_bucket_ ); bucket++ )
    {
        snprintf( buffer, sizeof( buffer ), "%6u", bucket );
        report += buffer;
    }

    report += "\nN: ";

    for ( uint32_t bucket = 0; bucket <= COLUMN_MAX( max_bucket_ ); bucket++ )
    {
        uint32_t score = buckets_[ bucket ];

        snprintf( buffer, sizeof( buffer ), "%6u", score );
        report += buffer;
    }

    report += "\n%: ";

    for ( uint32_t bucket = 0; bucket <= COLUMN_MAX( max_bucket_ ); bucket++ )
    {
        double fraction_of_total = ( 100.0 * ( double ) buckets_[ bucket ] ) / ( double ) total_score;

        snprintf( buffer, sizeof( buffer ), "%6s", format_percentage( fraction_of_total ).c_str() );
        report += buffer;
    }

    report += "\n";



        //double fraction_of_total   = ( 100.0 * ( double ) score ) / ( double ) total_score;
        //cumulative_fraction += fraction_of_total;
        
        //const char * marker = ( bucket == max_used_bucket_ ) ? "M" : " ";

        //char bucket_desc[ 20 ];

        //uint32_t bucket_from = ( bucket * ( uint32_t ) bucket_width_ );
        //uint32_t bucket_to   = ( ( ( bucket + 1 ) * ( uint32_t ) bucket_width_ ) - 1 );

        //if ( bucket_from == bucket_to )                                      
        //{
            //snprintf( bucket_desc, sizeof( bucket_desc ), "%7u", bucket_from );
        //}
        //else
        //{
            //snprintf( bucket_desc, sizeof( bucket_desc ), "%3u-%3u", bucket_from, bucket_to );
        //}

        //if ( bucket <= max_bucket_ )
        //{
            //snprintf( buffer, sizeof( buffer ), "%-7s   %s[%5u][%6s%%][%6s%%]| "
                                                //, bucket_desc
                                                //, marker
                                                //, score
                                                //, format_percentage( fraction_of_total ).c_str()
                                                //, format_percentage( cumulative_fraction ).c_str() );
            //report += buffer;

            //uint32_t bar_length = ( score * BAR_WIDTH ) / max_score;

            //if ( ( bar_length == 0 ) && ( score > 0 ) )
            //{
                //bar_length = 1;
            //}

            //snprintf( buffer, sizeof( buffer), "%s\n", std::string( bar_length, '-' ).c_str() );
            //report += buffer;
        //}
        //else
        //{
            //snprintf( buffer, sizeof( buffer), "%3.3s       %s[%5u][%6s%%][%6s%%]| ", ">"
                                                //, marker
                                                //, score
                                                //, format_percentage( fraction_of_total ).c_str()
                                                //, format_percentage( cumulative_fraction ).c_str() );
            //report += buffer;
        //}
    //}
 
    //snprintf( buffer, sizeof( buffer), "\n" );
    //report += buffer;
    
    //snprintf( buffer, sizeof( buffer), "Total:     [%5u]| \n", total_score );
    //report += buffer;

    return report;
}

std::string
C_distribution::format_percentage( double percentage )
{
    char buffer[ 50 ];
    snprintf( buffer, sizeof( buffer ), "%2.2f", percentage );

    return std::string( buffer );
}

}
