#!/usr/bin/perl
# tmpl2c.pl,v 1.3 1994/02/14 10:47:08 shigeya Exp
#
# Convert template file to C string, prepared for distribute
# Copyright(c)1993 1994 Shigeya Suzuki
#
# jcode.pl included for Japanese handling.

&jcode'init();

while (<>) {
    next if /^#/ || /^\s*$/;

    if (/^@\s+(.*)/) {
	$kitname = $1;
	print "char $kitname\[\] = \"\\\n";
	while (<>) {
	    last if /^@/;
	    &jcode'convert(*_, "euc");
	    s/"/\\"/g;
	    s/$/\\n\\/;
	    print;
	}
	print "\";\n\n\n";
    }
    elsif (/^&\s+(.*)/) {
	print "struct mestab $1\[\] = {\n";
	while (<>) {
	    last if /^&/;
	    ($pat, $name, $conversion) = split;
	    $conversion = 0 if $conversion eq "" || $conversion eq "none";
	    print "\t{ \"$pat\", $name, $conversion}, \n";
	}
	print "\t{0, 0, 0}\n};\n\n";
    }
}

package jcode;
;######################################################################
;#
;# jcode.pl: Japanese character code conversion library
;#
;# Copyright (c) 1991,1992 Software Research Associates, Inc.
;#	Original by srekcah@sra.co.jp, Feb 1992
;#
;#	Maintained by Kazumasa Utashiro <utashiro@sra.co.jp>
;#	Software Research Associates, Inc., Japan
;#
;; $rcsid = q!Id: jcode.pl,v 1.9 1994/02/14 06:16:29 utashiro Exp !;
;#
;######################################################################
;#
;# INTERFACE:
;#
;#	&jcode'getcode(*line)
;#		Return 'jis', 'sjis', 'euc' or undef according to
;#		Japanese character code in $line.
;#
;#	&jcode'convert(*line, $ocode [, $icode])
;#		Convert the line in any Japanese code to specified
;#		code in second argument $ocode.  $ocode is one of
;#		'jis', 'sjis' or 'euc'.  Input code is recognized
;#		automatically from the line itself when $icode is not
;#		supplied.  $icode also can be specified, but xxx2yyy
;#		routine is more efficient when both codes are known.
;#
;#		It returns a list of pointer of convert subroutine and
;#		input code.  It means that this routine returns the
;#		input code of the line in scalar context.
;#
;#	&jcode'xxx2yyy(*line)
;#		Convert Japanese code from xxx to yyy.  xxx and yyy
;#		are one of "jis", "sjis" or "euc".  These subroutines
;#		return number of converted substrings.  So return
;#		value 0 means the line was not converted at all.
;#
;#	&jcode'jis_inout($in, $out)
;#		Set or inquire JIS start and end sequences.  Default
;#		is "ESC-$-B" and "ESC-(-B".  If you supplied only one
;#		character, "ESC-$" or "ESC-(" is added as a prefix
;#		for each character respectively.  Acutually "ESC-(-B"
;#		is not a sequence to end JIS code but a sequence to
;#		start ASCII code set.  So `in' and `out' are somewhat
;#		misleading.
;#
;#	&jcode'get_inout($string)
;#		Get JIS start and end sequences from $string.
;#
;#	$jcode'convf{'xxx', 'yyy'}
;#		The value of this associative array is pointer to the
;#		subroutine jcode'xxx2yyy().
;#
;#	---------------------------------------------------------------
;#
;#	&jcode'init()
;#		Initialize the variables used in other functions.  You
;#		don't have to call this when using jocde.pl by do or
;#		require.  Call it first if you embedded the jcode.pl
;#		in your script.
;#
;######################################################################
;#
;# SAMPLES
;#
;# Convert any Kanji code to JIS and print each line with code name.
;#
;#	while (<>) {
;#	    $code = &jcode'convert(*_, 'jis');
;#	    print $code, "\t", $_;
;#	}
;#	
;# Convert all lines to JIS according to the first recognized line.
;#
;#	while (<>) {
;#	    print, next unless /[\033\200-\377]/;
;#	    (*f, $icode) = &jcode'convert(*_, 'jis');
;#	    print;
;#	    defined(&f) || next;
;#	    while (<>) { &f(*_); print; }
;#	    last;
;#	}
;#
;# The safest way for converting to JIS
;#
;#	while (<>) {
;#	    ($matched, $code) = &jcode'getcode(*_);
;#	    print, next unless (@buf || $matched);
;#	    push(@readahead, $_);
;#	    next unless $code;
;#	    eval "&jcode'${code}2jis(*_), print while (\$_ = shift(\@buf));";
;#	    eval "&jcode'${code}2jis(*_), print while (\$_ = <>);";
;#	    last;
;#	}
;#		
;######################################################################

;#
;# Call initialize function if not called yet.  This sounds strange
;# but this makes easy to embed the jcode.pl in the script.  Call
;# &jcode'init at the beginning of the script in that case.
;#
&init unless defined $version;

;#
;# Initialize variables.
;#
sub init {
    ($version) = ($rcsid =~ /,v ([\d.]+)/);
    $re_sjis_c = '[\201-\237\340-\374][\100-\176\200-\374]';
    $re_sjis_s = "($re_sjis_c)+";
    $re_euc_c  = '[\241-\376][\241-\376]';
    $re_euc_s  = "($re_euc_c)+";
    $re_jin    = '\033\$[\@B]';
    $re_jout   = '\033\([BJ]';
    &jis_inout("\033\$B", "\033(B");

    for $from ('jis', 'sjis', 'euc') {
	for $to ('jis', 'sjis', 'euc') {
	    eval "\$convf{$from, $to} = *${from}2${to};";
	}
    }
}

;#
;# Set JIS in and out final characters.
;#
sub jis_inout {
    $jin = shift || $jin;
    $jout = shift || $jout;
    $jin = "\033\$".$jin if length($jin) == 1;
    $jout = "\033\(".$jout if length($jout) == 1;
    ($jin, $jout);
}

;#
;# Get JIS in and out sequences from the string.
;#
sub get_inout {
    local($jin, $jout);
    $_[$[] =~ /$re_jin/o && ($jin = $&);
    $_[$[] =~ /$re_jout/o && ($jout = $&);
    ($jin, $jout);
}

;#
;# Character code recognition
;#
sub getcode {
    local(*_) = @_;
    return undef unless /[\033\200-\377]/;
    return 'jis' if /$re_jin|$re_jout/o;

    local($sjis, $euc);
    $sjis += length($&) while /$re_sjis_s/go;
    $euc  += length($&) while /$re_euc_s/go;
    (&max($sjis, $euc), ('euc', undef, 'sjis')[($sjis<=>$euc) + $[ + 1]);
}
sub max { $_[ $[ + ($_[$[] < $_[$[+1]) ]; }

;#
;# Convert any code to specified code
;#
sub convert {
    local(*_, $ocode, $icode) = @_;
    return (undef, undef) unless $icode = $icode || &getcode(*_);
    $ocode = 'jis' unless $ocode;
    local(*convf) = $convf{$icode, $ocode};
    do convf(*_);
    (*convf, $icode);
}

;#
;# JIS to JIS
;#
sub jis2jis {
    local(*_) = @_;
    s/$re_jin/$jin/go;
    s/$re_jout/$jout/go;
}

;#
;# SJIS to JIS
;#
sub sjis2jis {
    local(*_) = @_;
    s/$re_sjis_s/&_sjis2jis($&)/geo;
}
sub _sjis2jis {
    local($_) = @_;
    s/../$s2e{$&}||&s2e($&)/geo;
    tr/\241-\376/\041-\176/;
    $jin . $_ . $jout;
}

;#
;# EUC to JIS
;#
sub euc2jis {
    local(*_) = @_;
    s/$re_euc_s/&_euc2jis($&)/geo;
}
sub _euc2jis {
    local($_) = @_;
    tr/\241-\376/\041-\176/;
    $jin . $_ . $jout;
}

;#
;# JIS to EUC
;#
sub jis2euc {
    local(*_) = @_;
    s/$re_jin([!-~]*)$re_jout/&_jis2euc($1)/geo;
}
sub _jis2euc {
    local($_) = @_;
    tr/\041-\176/\241-\376/;
    $_;
}

;#
;# JIS to SJIS
;#
sub jis2sjis {
    local(*_) = @_;
    s/$re_jin([!-~]*)$re_jout/&_jis2sjis($1)/geo;
}
sub _jis2sjis {
    local($_) = @_;
    tr/\041-\176/\241-\376/;
    s/../$e2s{$&}||&e2s($&)/ge;
    $_;
}

;#
;# SJIS to EUC
;#
sub sjis2euc {
    local(*_) = @_;
    s/$re_sjis_c/$s2e{$&}||&s2e($&)/geo;
}
sub s2e {
    ($c1, $c2) = unpack('CC', $code = shift);
    if ($c2 >= 0x9f) {
	$c1 = $c1 * 2 - ($c1 >= 0xe0 ? 0xe0 : 0x60);
	$c2 += 2;
    } else {
	$c1 = $c1 * 2 - ($c1 >= 0xe0 ? 0xe1 : 0x61);
	$c2 += 0x60 + ($c2 < 0x7f);
    }
    $s2e{$code} = pack('CC', $c1, $c2);
}

;#
;# EUC to SJIS
;#
sub euc2sjis {
    local(*_) = @_;
    s/$re_euc_c/$e2s{$&}||&e2s($&)/geo;
}
sub e2s {
    ($c1, $c2) = unpack('CC', $code = shift);
    if ($c1 % 2) {
	$c1 = ($c1>>1) + ($c1 < 0xdf ? 0x31 : 0x71);
	$c2 -= 0x60 + ($c2 < 0xe0);
    } else {
	$c1 = ($c1>>1) + ($c1 < 0xdf ? 0x30 : 0x70);
	$c2 -= 2;
    }
    $e2s{$code} = pack('CC', $c1, $c2);
}

;#
;# SJIS to SJIS, EUC to EUC
;#
sub sjis2sjis { 0; }
sub euc2euc { 0; }

1;
