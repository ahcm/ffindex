#!/usr/bin/env perl

use strict;
use warnings;
use FFindex;

my $ffindex_db = new FFindex("../src/data.ffdata", "../src/data.ffindex");

print "by index:\n";
my $max = $ffindex_db->get_num_entries();
for(my $i = 0; $i < $max; $i++)
{
  my @entry =  @{$ffindex_db->get_entry_by_index($i)};
  print join("\t", @entry), "\n";

  print $ffindex_db->get_data_by_index($i);
}

print "by name:\n";
foreach my $name ("a","b","c")
{
  my @entry =  @{$ffindex_db->get_entry_by_name($name)};
  print join("\t", @entry), "\n";

  print $ffindex_db->get_data_by_name($name);
}
