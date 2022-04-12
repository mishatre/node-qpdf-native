
#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include "nan.h"

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFPageObjectHelper.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QUtil.hh>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <qpdf/Buffer.hh>

using namespace v8;
using namespace node;