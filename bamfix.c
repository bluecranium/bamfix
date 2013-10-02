/* Fix Bam QNAME Fields by removing /1 and /2 at the end*/

#include <stdio.h>
#include "sam.h"

// Given an alignment and a new qname, allocate space for this qname to be copied in later
// This code is inspired by pysam_bam_update() from pysam_util.c in pysam (http://code.google.com/p/pysam/)
bam1_t * allocqname(bam1_t * b, char * newname)
{
	size_t l_qname1 = b->core.l_qname;
	size_t l_qname2 = strlen(newname)+1;
	uint8_t * qname_ptr = bam1_qname(b);
	int diff = l_qname2-l_qname1;
	int new_size;
	size_t offset;

	// If there isn't any size different, we have no need to change anything
	if (diff == 0)
	{
		return b;
	}

	// Otherwise, calculate our new size and offset for qname
	new_size = diff + b->data_len;
	offset = qname_ptr - b->data;

	// If our new size if bigger than what we have allocated already, allocate more
	if (b->m_data < new_size)
    {
		b->m_data = new_size;
 		kroundup32(b->m_data);
 		b->data = (uint8_t*)realloc(b->data, b->m_data);
    }

	// Update our pointer to qname
    qname_ptr = b->data + offset;
  
	//printf("dest=%p, src=%p, n=%i\n", qname_ptr+l_qname2, qname_ptr+l_qname1, b->data_len - (offset+l_qname1));
	
	// Copy from our old to new data block
	memmove(qname_ptr+l_qname2, qname_ptr+l_qname1, b->data_len - (offset + l_qname1));
    
	// Set the length for the whole variable data block
	b->data_len = new_size;
      
	return b;
}

//Given an alignment, set newname as the QNAME
bam1_t * setqname(bam1_t * b, char * newname)
{
	size_t newlen = strlen(newname)+1;
	// Allocate space for new QNAME
	b = allocqname(b, newname);
	// Set new QNAME length
	b->core.l_qname = strlen(newname)+1;
	// Copy QNAME value
	strncpy(bam1_qname(b), newname, strlen(newname)+1);
	
	return b;
}

//Given a pointer to a string, return a pointer to a new string that is the requested substring
//Start and end should be the 0-based indices of the desired substring
//Function appends the null terminator to the substring
char * substring(char * str, size_t start, size_t end)
{
	size_t size = (end-start)+1;
	char * str_sub = (char *) malloc(size+1);
	strncpy(str_sub, str+start, size);
	str_sub[size] = '\0';
	return str_sub;
}

//main()
//Requires in.bam and out.bam as command line arguments
int main(int argc, char *argv[])
{
	samfile_t *in;
	samfile_t *out;
 	bam1_t *b;

	// Check that we have 2 arguments
	if (argc < 3)
	{ 
		fprintf(stderr, "Usage: bamfix <in.bam> <out.bam>\n");
		return 1;  
	}  

	// Open input BAM (read only)
	in = samopen(argv[1], "rb", 0);  
	if (in == 0)
	{  
		fprintf(stderr, "Error opening %s\n", argv[1]);  
		return 1;
	}

	// Open filehandle to output bam
	out = samopen(argv[2], "wb", in->header);

	// Initialize an empty alignment variable to read the first alignment into
	b = bam_init1();

	// Loop for each alignment in the input BAM, reading the alignment into b each time
	int i=0;
	int j=0;
	while(samread(in, b) > 0)
	{
		// Increment line counter
		i++;
		j++;
		if(j==1e6)
		{
			j=0;
			printf("Done with %i lines\n", i);
		}
		//printf("QNAME: %s\tLEN: %i\tDLEN: %i\tMLEN: %i\n", bam1_qname(b), b->core.l_qname, b->data_len, b->m_data);
		//printf("Calling substring with %p, %i, %i\n", bam1_qname(b), 0, b->core.l_qname-1-5);
		// Get substring on QNAME
		char * ptr_str = substring(bam1_qname(b), 0, b->core.l_qname-1-3);
		//printf("QNAME: %s\tLEN: %i\n", ptr_str, strlen(ptr_str)+1);
		b = setqname(b, ptr_str);
		// Free up space from the substring we allocated memory for
		free(ptr_str);
		//printf("QNAME: %s\tLEN: %i\tDLEN: %i\tMLEN: %i\n\n", bam1_qname(b), b->core.l_qname, b->data_len, b->m_data);
		// Write the altered alignment to the output BAM
		samwrite(out, b);
 		bam_destroy1(b);
		b = bam_init1();
	}

	// Close filehandles
	bam_destroy1(b);
	samclose(in);
	samclose(out);
	return 0;
}

