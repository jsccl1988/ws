#include <wspp/util/forms.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>


using namespace std ;

namespace wspp {

class InputField: public FormField {
public:
    InputField(const string &name, const string &type): FormField(name), type_(type) {}

    void fillData(Variant::Object &) const override;
private:
    string type_ ;
};

class SelectField: public FormField {
public:
    SelectField(const string &name, boost::shared_ptr<OptionsModel> options, bool multi): FormField(name), options_(options), multiple_(multi) {
        addValidator([&](const string &val, FormField &v) {
            Dictionary options = options_->fetch() ;
            if ( multiple_ ) {
                boost::char_separator<char> sep(" ");
                boost::tokenizer<boost::char_separator<char> > tokens(val, sep);

                for( const string &s: tokens ) {
                    if ( !options.contains(s) ) {
                        v.addErrorMsg("Supplied value: " + val + " is not in option list") ;
                        return false ;
                    }
                }
            }
            else {
               if ( !options.contains(val) ) {
                    v.addErrorMsg("Supplied value: " + val + " is not in option list") ;
                    return false ;
                }
            }
            v.value(val) ;
            return true ;
        }) ;

    }

    void fillData(Variant::Object &) const override;

private:
    bool multiple_ ;
    boost::shared_ptr<OptionsModel> options_ ;
};

class CheckBoxField: public FormField {
public:
    CheckBoxField(const string &name, bool is_checked = false): FormField(name), is_checked_(is_checked) {

    }

    void fillData(Variant::Object &res) const override {
        FormField::fillData(res) ;
        if ( is_checked_ ) res.insert({"checked", "checked"}) ;
        res.insert({"template", "checkbox"}) ;
    }
private:
    bool is_checked_ = false ;
};

static bool validate_required_arg(const string &val, FormField &field) {
    if ( val.empty() ) {
        field.addErrorMsg("Required field missing") ;
        return false ;
    }

    field.value(val) ;
    return true ;
}

FormField::Validator FormField::requiredArgValidator = validate_required_arg ;

void FormField::fillData(Variant::Object &res) const {

    if ( !label_.empty() ) res.insert({"label", label_}) ;
    if ( !name_.empty() ) res.insert({"name", name_}) ;

    if ( !value_.empty() ) res.insert({"value", value_}) ;
    else if ( !initial_value_.empty() ) res.insert({"value", initial_value_}) ;

    res.insert({"id", id_}) ;

    if ( !place_holder_.empty() ) res.insert({"placeholder", place_holder_}) ;
    if ( !help_text_.empty() ) res.insert({"help_text", help_text_}) ;
    res.insert({"required", required_}) ;
    res.insert({"disabled", disabled_}) ;
    if ( !extra_classes_.empty() ) res.insert({"extra_classes", extra_classes_}) ;
    if ( !extra_attrs_.empty() ) res.insert({"extra_attrs", Variant::fromDictionary(extra_attrs_)}) ;
    if ( !error_messages_.empty() ) res.insert({"errors", Variant::Object{{"messages", Variant::fromVector(error_messages_)}}}) ;
}

bool FormField::validate(const string &value)
{
    // normalize passed value

    string n_value = normalizer_ ? normalizer_(value) : value ;

    // call validators

    for( auto &v: validators_ ) {
        if ( ! v(n_value, *this) ) return false ;
    }

    // store value

    value_ = n_value ;
    return true ;
}

void InputField::fillData(Variant::Object &base) const
{
    FormField::fillData(base) ;
    base.insert({"type", type_}) ;
    base.insert({"template", "input"}) ;
}


void SelectField::fillData(Variant::Object &base) const
{
    FormField::fillData(base) ;
    base.insert({"multiselect", multiple_}) ;
    base.insert({"template", "select"}) ;
    if ( options_ ) {
        Dictionary options = options_->fetch() ;
        if ( !options.empty() ) base.insert({"options", Variant::fromDictionaryAsArray(options)}) ;
    }
}


Form::Form(const std::string &prefix, const string &suffix): field_prefix_(prefix), field_suffix_(suffix) {

}

FormField &Form::input(const string &name, const string &type)
{
    const auto &it = fields_.insert({name, boost::make_shared<InputField>(name, type)}) ;
    assert(it.second) ;
    auto &field = it.first->second ;
    field->id(field_prefix_ + name + field_suffix_) ;
    field->count_ = fields_.size() ;
    return *field ;
}


FormField &Form::select(const string &name, boost::shared_ptr<OptionsModel> options, bool multi) {
    const auto &it = fields_.insert({name, boost::make_shared<SelectField>(name, options, multi)}) ;
    assert(it.second) ;
    auto &field = it.first->second ;
    field->id(field_prefix_ + name + field_suffix_) ;
    field->count_ = fields_.size() ;
    return *field ;
}

FormField &Form::checkbox(const string &name, bool is_checked)
{
    const auto &it = fields_.insert({name, boost::make_shared<CheckBoxField>(name, is_checked)}) ;
    assert(it.second) ;
    auto &field = it.first->second ;
    field->id(field_prefix_ + name + field_suffix_) ;
    field->count_ = fields_.size() ;
    return *field ;

}

Variant::Object Form::data() const
{
    Variant::Object form_data ;
    if ( !errors_.empty() )
        form_data.insert({"global_errors", Variant::Object{{"messages", Variant::fromVector(errors_)}}}) ;

    vector<boost::shared_ptr<FormField>> fieldv ;

    for( const auto &p: fields_ )
        fieldv.push_back(p.second) ;

    std::sort(fieldv.begin(), fieldv.end(), [&](const boost::shared_ptr<FormField> &f1, const boost::shared_ptr<FormField> &f2) { return f1->count_ < f2->count_ ; }) ;

    Variant::Array field_data_list ;
    for( const auto &p: fieldv ) {
        Variant::Object field_data ;
        p->fillData(field_data) ;
        if ( is_valid_ )
            field_data.insert({"value", p->value_}) ;
        else if ( !p->initial_value_.empty() )
            field_data.insert({"value", p->initial_value_}) ;

        field_data_list.push_back(field_data) ;
    }

    form_data.insert({"fields", field_data_list}) ;

    return form_data ;
}

string Form::getValue(const string &field_name)
{
    const auto &it = fields_.find(field_name) ;
    assert ( it != fields_.end() ) ;
    return it->second->value_ ;
}

bool Form::validate(const Dictionary &vals) {
    bool failed = false ;
    for( const auto &p: fields_ ) {
        if ( vals.contains(p.first) ) {

            bool res = p.second->validate(vals.get(p.first)) ;
            if ( !res ) failed = true ;
        }
    }
    is_valid_ = !failed ;
    return is_valid_ ;
}

void Form::init(const Dictionary &vals) {
    for( const auto &p: vals ) {
        auto it = fields_.find(p.first) ;
        if ( it != fields_.end() ) {
            it->second->value(p.second) ;
        }
    }
}


}
