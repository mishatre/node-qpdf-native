#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif // BUILDING_NODE_EXTENSION

#define POINTERHOLDER_TRANSITION 0

#if _MSC_VER && _MSC_VER < 1900
#define snprintf _snprintf
#endif

#include "qpdf.hh"
#include <list>
#include <sstream>
#include <string.h>
#include <exception>

inline Local<Value> WrapPointer(char *ptr, size_t length) {
    Nan::EscapableHandleScope scope;
    return scope.Escape(Nan::CopyBuffer(ptr, length).ToLocalChecked());
}
inline Local<Value> WrapPointer(char *ptr) {
    return WrapPointer(ptr, 0);
}

struct q_ctx_base {
    Nan::Callback * callback;
    std::string error;

    char* inData;
    size_t inLength;

    ::Buffer* outData;

    virtual ~q_ctx_base() {}
};

struct overlay_q_ctx: q_ctx_base {
    char* overlayData;
    size_t overlayLength;
    overlay_q_ctx() {}
};

void DoOverlay(uv_work_t* req) {

    overlay_q_ctx* context = static_cast<overlay_q_ctx*>(req->data);

    QPDF inpdf;
    inpdf.processMemoryFile("input", context->inData, context->inLength);

    QPDF stamppdf;
    stamppdf.processMemoryFile("overlay", context->overlayData, context->overlayLength);

    // Get first page from other file
    QPDFPageObjectHelper stamp_page_1 = QPDFPageDocumentHelper(stamppdf).getAllPages().at(0);
    // Convert page to a form XObject
    QPDFObjectHandle foreign_fo = stamp_page_1.getFormXObjectForPage();
    // Copy form XObject to the input file
    QPDFObjectHandle stamp_fo = inpdf.copyForeignObject(foreign_fo);

    // For each page
    std::vector<QPDFPageObjectHelper> pages = QPDFPageDocumentHelper(inpdf).getAllPages();
    for(std::vector<QPDFPageObjectHelper>::iterator iter = pages.begin(); iter != pages.end(); ++iter) {

        QPDFPageObjectHelper& ph = *iter;

        // Find a unique resource name for the new form XObject
        QPDFObjectHandle resources = ph.getAttribute("/Resources", true);
        int min_suffix = 1;
        std::string name = resources.getUniqueResourceName("/Fx", min_suffix);

        // // Generate content to place the form XObject centered within
        // // destination page's trim box.
        // QPDFMatrix m;
        std::string content = ph.placeFormXObject(stamp_fo, name, ph.getTrimBox().getArrayAsRectangle());
        if(!content.empty()) {
            // Append the content to the page's content. Surround the
            // original content with q...Q to the new content from the
            // page's original content.
            resources.mergeResources(QPDFObjectHandle::parse("<< /XObject << >> >>"));
            resources.getKey("/XObject").replaceKey(name, stamp_fo);
            ph.addPageContents(QPDFObjectHandle::newStream(&inpdf, "q\n"), true);
            ph.addPageContents(QPDFObjectHandle::newStream(&inpdf, "\nQq\n" + content), false);
        }

        // Copy the annotations and form fields from the original page
        // to the new page. For more efficiency when copying multiple
        // pages, we can create a QPDFAcroFormDocumentHelper and pass
        // it in. See comments in QPDFPageObjectHelper.hh for details.
        // ph.copyAnnotations(stamp_page_1, m);

    }

    QPDFWriter w(inpdf);
    w.setOutputMemory();
    w.write();

    context->outData = w.getBuffer();

}

// Make callback from overlay
void GeneratedBlobAfter(uv_work_t* req, int n) {
    Nan::HandleScope scope;

    q_ctx_base* context = static_cast<q_ctx_base*>(req->data);
    delete req;

    Local<Value> argv[2];

    if(!context->error.empty()) {
        argv[0] = Exception::Error(Nan::New<String>(context->error.c_str()).ToLocalChecked());
        argv[1] = Nan::Undefined();
    } else {
        argv[0] = Nan::Undefined();
        argv[1] = WrapPointer((char*) context->outData->getBuffer(), context->outData->getSize());
    }

    // std::cout << argv[1] << std::endl;

    Nan::TryCatch try_catch;

    Nan::AsyncResource resource("GeneratedBlobAfter");

    context->callback->Call(2, argv, &resource);

    delete context->callback;

    delete context;

    if(try_catch.HasCaught()) {
#if NODE_VERSION_AT_LEAST(0,12,0)
        Nan::FatalException(try_catch);
#else
        FatalException(try_catch);
#endif        
    }


}

NAN_METHOD(Overlay) {
    Nan::HandleScope();

    

    Local<Object> options = Local<Object>::Cast(info[0]);
    // Local<Object> overlayObj = Local<Object>::Cast(info[1]);

    Local<Object> inData = Local<Object>::Cast(Nan::Get(options, Nan::New<String>("inData").ToLocalChecked()).ToLocalChecked());
    Local<Object> overlayData = Local<Object>::Cast(Nan::Get(options, Nan::New<String>("overlayData").ToLocalChecked()).ToLocalChecked());

    if (inData->IsUndefined() || !Buffer::HasInstance(inData)) {
        return Nan::ThrowError("overlay()'s first argument should have \"inData\" key with a Buffer instance");
    }

    if (overlayData->IsUndefined() || !Buffer::HasInstance(overlayData)) {
        return Nan::ThrowError("overlay()'s second argument should have \"overlayData\" key with a Buffer instance");
    }

    overlay_q_ctx* context = new overlay_q_ctx();

    context->inData = Buffer::Data(inData);
    context->inLength = Buffer::Length(inData);

    context->overlayData = Buffer::Data(overlayData);
    context->overlayLength = Buffer::Length(overlayData);

    uv_work_t* req = new uv_work_t();
    req->data = context;
    
    context->callback = new Nan::Callback(Local<Function>::Cast(info[1]));

    uv_queue_work(uv_default_loop(), req, DoOverlay, (uv_after_work_cb)GeneratedBlobAfter);

    return;

}


void init(Local<Object> exports) {
    Nan::SetMethod(exports, "overlay", Overlay);
}

NODE_MODULE(qpdf, init)