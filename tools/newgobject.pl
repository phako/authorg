#!/usr/bin/perl -w

use strict;

my ($small, $parent) = @ARGV;

my $filename = $small;
$filename =~ s/_/-/g;

my $cap = uc $small;

my $camel = $small;
$camel =~ s/_(.)/\u$1/g;
$camel =~ s/^(.)/\u$1/g;

print "Generating $filename.c and $filename.h...\n";

my $header ="
/* Authorg - a GNOME DVD authoring tool
 *
 * Copyright (C) 2005 Jens Georg <mail\@jensge.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 */

#ifndef _&cap_H
#define _&cap_H

#define AUTHORG_TYPE_$cap (authorg_&small_get_type())
#define AUTHORG_$cap(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), AUTHORG_TYPE_$cap, Authorg$camel))
#define AUTHORG_&cap_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), AUTHORG_TYPE_$cap, Authorg&camelClass))
#define AUTHORG_IS_$cap(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AUTHORG_TYPE_$cap))
#define AUTHORG_IS_&cap_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), AUTHORG_TYPE_$cap))
#define AUTHORG_&cap_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), AUTHORG_TYPE_$cap, Authorg&camelClass))

typedef struct Authorg$camel Authorg$camel;
typedef struct Authorg&camelClass Authorg&camelClass;
typedef struct Authorg&camelPrivate Authorg&camelPrivate;

struct Authorg$camel {
		$parent parent;
		Authorg&camelPrivate *_priv;
};

struct Authorg&camelClass {
		&parentClass parent;
};

GType authorg_&small_get_type (void);
#endif /* _&cap_H */
";

$header =~ s/&camel/$camel/g;
$header =~ s/&cap/$cap/g;
$header =~ s/&parent/$parent/g;
$header =~ s/&small/$small/g;

open (HEADER, ">$filename.h");
print HEADER $header;
close (HEADER);

$parent =~ s/G_/G_TYPE_/;

$header= "
/* Authorg - a GNOME DVD authoring tool
 *
 * Copyright (C) 2005 Jens Georg <mail\@jensge.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 */

#include \"$filename.h\"

struct Authorg&camelPrivate
{
};

G_DEFINE_TYPE (Authorg$camel, authorg_$small, $parent);

static void
authorg_&small_init (Authorg$camel *$small)
{
	$small->_priv = g_new0(Authorg&camelPrivate, 1);
}

static void
authorg_&small_finalize (GObject *object)
{
	Authorg$camel $small = AUTHORG_$cap (object);

	g_return_if_fail (object != NULL);

	if (G_OBJECT_CLASS (authorg_&small_parent_class)->finalize)
		G_OBJECT_CLASS (authorg_&small_parent_class)->finalize (object);
}

static void
authorg_&small_class_init (Authorg&camelClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->finalize = authorg_&small_finalize;
}";

$header =~ s/&camel/$camel/g;
$header =~ s/&cap/$cap/g;
$header =~ s/&parent/$parent/g;
$header =~ s/&small/$small/g;

open (HEADER, ">$filename.c");
print HEADER $header;
close (HEADER);


