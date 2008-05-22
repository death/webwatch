#
# This script will generate a new WebWatch XML configuration from your
# old one. The new configuration will include all the site groups/sites
# you had in the old configuration. Parameters other than site's
# name/address aren't preserved through this script.
#
# Also note that this is one of my first Perl programs :D
#
#   25 Sep 2004 - Initial version.
#   11 Oct 2004 - Site notes are also converted.
#
use strict;
use warnings;

use XML::Simple;

die "This script needs a file named Old.xml (your old configuration).\n" unless -e "Old.xml";
die "This script will NOT overwrite WebWatch.xml, which currently exists.\n" if -e "WebWatch.xml";

my $oldconf = XMLin('Old.xml', forcearray => 1);
our $newconf = "<WebWatch>";

$newconf .= "<SiteItemGroup>"
         .  "<Name>Root</Name>";

foreach my $group (@{$oldconf->{Sites}}) {
    process_group($group);
}

$newconf .= "</SiteItemGroup>"
         .  "</WebWatch>\n";

open(NEWCONF, '>WebWatch.xml') or die $newconf;
print NEWCONF $newconf;
close NEWCONF;

die "New configuration written.\n";

sub process_group
{
    my $group = $_[0];

    $newconf .= "<SiteItemGroup>"
             .  "<Name>$group->{Name}</Name>";

    foreach my $site (@{$group->{Site}}) {
        process_site($site);
    }

    foreach my $subgroup (@{$group->{Sites}}) {
        process_group($subgroup);
    }

    $newconf .= "</SiteItemGroup>";
}

sub process_site
{
    my $site = $_[0];

    $newconf .= "<Site>"
             . "<Address>$site->{Address}->[0]</Address>"
             . "<UISettings>"
             . "<Name>$site->{Name}</Name>";

    if (defined $site->{Notes}) {
        $newconf .= "<Notes>$site->{Notes}->[0]</Notes>";
    }

    $newconf .= "</UISettings>"
             . "</Site>";
}
