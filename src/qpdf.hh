
#include <qpdf/QPDF.hh>
#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include "nan.h"

#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFPageObjectHelper.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/Buffer.hh>

using namespace v8;
using namespace node;