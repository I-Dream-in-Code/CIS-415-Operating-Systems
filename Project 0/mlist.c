#include "mlist.h"
#include "mentry.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

unsigned long SIZE = 32;

typedef struct node
{
	struct node *next;
	MEntry *me;
} Node;

typedef struct mlist
{
	int *hash_bucket_counter;
	Node **hash_table;
} Mlist;

/* ml_create - created a new mailing list */
MList *ml_create ( void )
{
	MList *p;
	p = ( MList * ) malloc ( sizeof ( MList ) );

	if ( p == NULL ) return NULL;

	else
	{
		p->hash_bucket_counter = malloc ( sizeof ( int ) * SIZE );

		if ( p->hash_bucket_counter == NULL )
		{
			free ( p );
			return NULL;
		}

		//initialize hash bucket counter to 0
		for ( int i = 0; i < SIZE; i++ )
		{
			p->hash_bucket_counter[i] = 0;
		}

		p->hash_table = malloc ( sizeof ( Node * ) *SIZE );

		if ( p->hash_table == NULL )
		{
			free ( p->hash_bucket_counter );
			free ( p );
			return NULL;
		}

		return p;
	}
}

/* ml_add - adds a new MEntry to the list;
 * returns 1 if successful, 0 if error (malloc)
 * returns 1 if it is a duplicate */
int ml_add ( MList **ml, MEntry *me )
{
	MList *p = *ml;
	unsigned long hash = me_hash ( me, SIZE );

	//if duplicate entry, ignore but return 1
	if ( ml_lookup ( p, me ) != NULL )
	{
		return 1;
	}

	else
	{
// 		/*if hash bucket exceeds 20-> double calloc
		if ( p->hash_bucket_counter[hash]  + 1 > 20 )
		{


			SIZE = SIZE * 2;

			//create temp hash table and counter
// 			MList *temp_hash_table = ml_create();
			Node **temp_h = malloc ( sizeof ( Node * ) *SIZE );

			if ( temp_h == NULL )
			{
				ml_destroy ( p );
				return 0;
			}

			int *cnt = malloc ( sizeof ( int ) * SIZE );

			if ( cnt == NULL )
			{
				free ( temp_h );
				ml_destroy ( p );
				return 0;
			}

			for ( int i = 0; i < SIZE; i++ )
			{
				cnt[i] = 0;
			}

			//go through current hash table and recalculate hash
			for ( int i = 0; i < SIZE / 2; i++ )
			{
				if ( p->hash_table[i] != NULL )
				{
					Node *q = p->hash_table[i];

					while ( q != NULL )
					{

						Node *nextNode = q->next;
						unsigned long new_hash = me_hash ( q->me, SIZE );
						q->next = temp_h[new_hash];
						temp_h[new_hash] = q;
						q = nextNode;
						cnt[new_hash] += 1;
					}

				}

			}
	for ( int i = 0; i < SIZE/2; i++ )
			{

				Node *n = p->hash_table[i];

				if ( n != NULL )
				{
					while ( n->next != NULL )
					{

						Node *r = n->next;
						me_destroy ( n->me );
						free ( n );
						n = r;
					}
				}

			}



			free ( p->hash_bucket_counter );
			free ( p->hash_table );

			p->hash_bucket_counter = cnt;
			p->hash_table = temp_h;

		}

		//if calloc failed return 0

// 		printf("%lu",SIZE);
		unsigned long new_hash = me_hash ( me, SIZE );

		Node *n = p->hash_table[new_hash];

		Node *entry = malloc ( sizeof ( Node ) );


		if ( entry == NULL )
		{

			ml_destroy ( p );
			return 0;
		}

		if ( n == NULL )
		{

			entry->next = NULL;
			entry->me = me;
			p->hash_table[new_hash] = entry;
			p->hash_bucket_counter[new_hash] += 1;

		}

		else
		{
			while ( n->next != NULL )
			{
				n = n->next;
			}

			entry->next = NULL;
			entry->me = me;
			n->next = entry;
			p->hash_bucket_counter[new_hash] += 1;


		}
	}

	return 1;

}

/* ml_lookup - looks for MEntry in the list, returns matching entry or NULL */
MEntry *ml_lookup ( MList *ml, MEntry *me )
{
	int hash = me_hash ( me, SIZE );

	//start at ml->hash_table[hash]
	Node *n = ml->hash_table[hash];

	for ( Node *q = ml->hash_table[hash]; q != NULL; q = q->next )
	{
		if ( q != NULL )
		{
			if ( me_compare ( me, q->me ) == 0 )
				return q->me;
		}
	}

	return NULL;
}
/* ml_destroy - destroy the mailing list */
void ml_destroy ( MList *ml )
{

	for ( int i = 0; i < SIZE; i++ )
	{

		Node *n = ml->hash_table[i];

		if ( n != NULL )
		{
			while ( n != NULL )
			{

				Node *r = n->next;
				me_destroy ( n->me );
				free ( n );
				n = r;
			}
		}

	}

	free ( ml->hash_table );
	free ( ml->hash_bucket_counter );
	free ( ml );
}
