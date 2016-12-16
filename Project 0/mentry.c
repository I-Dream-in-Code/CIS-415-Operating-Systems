#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include "mentry.h"

MEntry *me_get ( FILE *FD )
{
	MEntry *me =  malloc ( sizeof ( MEntry) );
	
	if ( me == NULL )
	{

		return NULL;
	}

	char *line_1;
	char *line_2;
	char *line_3;

	for ( int i = 0; i < 3; i++ )
	{
		char *buffer = malloc ( sizeof ( char ) * 128 );

		//read next line


		//capture surname
		if ( i == 0 )
		{
			if ( fgets ( buffer, 128, FD ) == NULL )
			{
				free ( me );
				free ( buffer );
				return NULL;
			}


			//copy line 1 without null terminator for full address
			line_1 = malloc ( strlen ( buffer ) );

			if ( line_1 == NULL )
			{
				free ( buffer );
				free ( me );

				return NULL;
			}

			snprintf ( line_1, strlen ( buffer ), "%s", buffer );
			//put surname in me->surname
			char *surname;
			surname = strtok ( buffer, "," );

			me->surname = malloc ( strlen ( surname ) + 1 );

			if ( me->surname == NULL )
			{
				free ( line_1 );
				free ( buffer );
				free ( me );

				return NULL;
			}

			snprintf ( me->surname, strlen ( surname ) + 1, "%s", surname );

			//convert to lowercase
			for ( unsigned int i = 0; i < strlen ( me->surname ); i++ )
			{
				me->surname[i] = tolower ( me->surname[i] );
			}
		}

		//capture house #
		if ( i == 1 )
		{
			if ( fgets ( buffer, 128, FD ) == NULL )
			{
				free ( line_1 );
				free ( me->surname );
				free ( buffer );
				free ( me );
				return NULL;
			}

			//save line_2 for me->full_address
			line_2 = malloc ( strlen ( buffer ) );

			if ( line_2 == NULL )
			{
				free ( me->surname );
				free ( line_1 );
				free ( buffer );
				free ( me );

				return NULL;
			}

			snprintf ( line_2, strlen ( buffer ), "%s", buffer );
			//save house #
			char *house_num_string;
			
			
			int digit=0;
			if(isdigit(line_2[0])){
				digit =1;
			}
			if (digit==1)
			{	

				house_num_string = strtok ( buffer, " " );
				
				me->house_number = atoi ( house_num_string );
			}
			else {
				
				me->house_number=0;
			}
			
		}

		if ( i == 2 )
		{
			if ( fgets ( buffer, 128, FD ) == NULL )
			{
				free ( line_1 );
				free ( line_2 );
				free ( me->surname );
				free ( buffer );
				free ( me );

				return NULL;
			}

			//copy line 3 without null terminator for full address
			line_3 = malloc ( strlen ( buffer ) + 1 );

			if ( line_3 == NULL )
			{
				free ( line_1 );
				free ( line_2 );
				free ( me->surname );
				free ( buffer );
				free ( me );

				return NULL;
			}

			snprintf ( line_3, strlen ( buffer ) + 1, "%s", buffer );

			char *temp = strtok ( buffer, "\n" );
			char *z;

			z = malloc ( strlen ( &temp[strlen ( temp ) - 5] ) );

			if ( z == NULL )
			{
				free ( line_1 );
				free ( line_2 );
				free ( line_3 );
				free ( me->surname );
				free ( me );
				free ( buffer );

				return NULL;
			}

			strcpy ( z, &temp[strlen ( temp ) - 5] );

			me->zipcode = malloc ( strlen ( z ) + 1 );

			if ( me->zipcode == NULL )
			{
				free ( line_1 );
				free ( line_2 );
				free ( line_3 );
				free ( me->surname );
				free ( z );
				free ( me );
				free ( buffer );

				return NULL;
			}

			snprintf ( me->zipcode, strlen ( z ) + 1, "%s", z );
			free ( z );
		}

		free ( buffer );
	}

	//concat full address

	me->full_address = malloc ( strlen ( line_1 ) + strlen ( line_2 ) + strlen ( line_3 ) + ( sizeof ( char ) * 3 ) + 1 );

	if ( me->full_address == NULL )
	{
		free ( line_1 );
		free ( line_2 );
		free ( line_3 );
		free ( me->surname );
		free ( me->zipcode );
		free ( me );

		return NULL;
	}

	strcpy ( me->full_address, line_1 );
	strcat ( me->full_address, "\n" );
	strcat ( me->full_address, line_2 );
	strcat ( me->full_address, "\n" );
	strcat ( me->full_address, line_3 );

	
	free ( line_1 );
	free ( line_2 );
	free ( line_3 );

	return me;

};

unsigned long me_hash ( MEntry *me, unsigned long size )
{
	//convert to ascii encoding

	char house_number_string[1024];
	char *s = malloc ( strlen ( me->surname ) + strlen ( house_number_string ) + strlen ( me->zipcode ) );
	sprintf(house_number_string,"%i",me->house_number);
	
	strcpy ( s, me->surname );

	strcat ( s, house_number_string );

	strcat ( s, me->zipcode );

	unsigned hashval = 0;
	
	char ss[1024];
	strncpy ( ss, s, strlen ( s ) + 1 );
	  
	free ( s );

	for ( unsigned int i = 0; strncmp(&ss[i],"\0",1)!=0; i++ )
	{

		hashval = ss[i] + 31 * hashval;
	}
	return hashval % size;
}
void me_print ( MEntry *me, FILE *fd )
{

	fprintf ( fd, "%s", ( me->full_address ) );
}

int me_compare ( MEntry *me1, MEntry *me2 )
{
	//if all equal return 0

	int answer;

	answer = strcmp ( me1->surname, me2->surname );

	if ( answer == 0 )
	{

		answer = strcmp ( me1->zipcode, me2->zipcode );

		if ( answer == 0 )
		{

			if ( me1->house_number != me2->house_number )
			{

				answer = me1->house_number - me2->house_number;
			}
		}
	}
 
	return answer;

}

void me_destroy ( MEntry *me )
{
	free ( me->full_address );
	free ( me->zipcode );
	free ( me->surname );
	free ( me );
}
