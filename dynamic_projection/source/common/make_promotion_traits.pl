#!/usr/bin/perl -w
#
# make_promotion_traits.pl: create a heade file with promotion_traits classes
#
@types = ('bool', 'char', 'short', 'int', 'long',
	  'unsigned char', 'unsigned short',
	  'unsigned int', 'unsigned long', 
	  'float', 'double');
#@gnu_types = ('long long', 'unsigned long long', 'long double');

sub make_c_generator(\@){
    my @types = @{$_[0]};
    print join("\n", @types);
    foreach $type1 (@types) {
	foreach $type2 (@types) {
	    print <<"EOF";
template<>
struct promotion_traits <$type1, $type2> {
  
};
EOF
	}
    }

}


#
# create subroutines
#
print <<"EOF";
template<typename T1, typename T2>
make_c_generator(@types);

#
# create main program
#
print <<"EOF";
#include <typeinfo>
int main(){
    
    return 0;
}
EOF


