#
# A simple perl native Module interfacing FFindex
# written by Andreas Hauser <andreas-source@creative-memory.de
#

package FFindex;

use Sys::Mmap;

sub new
{
    my $class = shift;
    my $self = {
        ffdata_filename   => shift,
        ffindex_filename  => shift,
        mode              => shift,
        ffdata            => undef,
        ffdata_file       => undef,
        ffindex_file      => undef,
        ffindex           => [],
        ffindex_hash      => {},
        n_entries         => []
    };

    bless $self, $class;

    #new Sys::Mmap $self->{ffdata}, 0, $self->{ffdata_filename} or die $!;
    open($self->{ffdata_file}, "<", $self->{ffdata_filename});
    mmap($self->{ffdata}, 0, PROT_READ, MAP_SHARED, $self->{ffdata_file}) or die $!;
    open($self->{ffindex_file}, "<", $self->{ffindex_filename});

    my $fh = $self->{ffindex_file};
    my $i = 0;
    while( my $line = <$fh>)
    {   
      chomp $line;
      my ($name, $offset, $len) = split(/\t/, $line);
      $self->{ffindex}[$i] = [$name, $offset, $len];
      $self->{ffindex_hash}{$name} = $self->{ffindex}[$i];
      $i++;
    }
    $self->{n_entries} = $i;

    return $self;
}


sub get_num_entries()
{
  my( $self ) = @_;
  return $self->{n_entries};
}


sub get_entry_by_index()
{
  my( $self, $index ) = @_;
  return $self->{ffindex}[$index];
}


sub get_entry_by_name()
{
  my( $self, $qname ) = @_;
  return $self->{ffindex_hash}{$qname};
}


sub get_data_by_index()
{
  my( $self, $index ) = @_;
  my ($name, $offset, $len) = @{$self->{ffindex}[$index]};
  return substr($self->{ffdata}, $offset, $len - 1);
}


sub get_data_by_name()
{
  my( $self, $qname ) = @_;
  my ($name, $offset, $len) = @{$self->{ffindex_hash}{$qname}};
  return substr($self->{ffdata}, $offset, $len - 1);
}



sub DESTROY
{
  close($self->{ffdata_file});
  close($self->{ffindex_file});
  $self->{ffindex} = undef;
}


1;
