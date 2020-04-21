#define LOG_TAG "FontSelector"

#include <jni.h>

#include <array>
#include <string>
#include <vector>

#include <android/log.h>
#include <android/font.h>
#include <android/font_matcher.h>
#include <android/system_fonts.h>

#ifndef NELEM
# define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

namespace {
    class ScopedUtfChars {
    public:
        ScopedUtfChars(JNIEnv* env, jstring s) : mEnv(env), mString(s) {
            if (s == nullptr) {
                mUtfChars = nullptr;
                mSize = 0;
            } else {
                mUtfChars = mEnv->GetStringUTFChars(mString, nullptr);
                mSize = mEnv->GetStringUTFLength(mString);
            }
        }

        ~ScopedUtfChars() {
            if (mUtfChars) {
                mEnv->ReleaseStringUTFChars(mString, mUtfChars);
            }
        }

        const char* c_str() const {
            return mUtfChars;
        }

        size_t size() const {
            return mSize;
        }

    private:
        JNIEnv* mEnv;
        jstring mString;
        const char* mUtfChars;
        size_t mSize;
    };

    class ScopedStringChars {
    public:
        ScopedStringChars(JNIEnv* env, jstring s) : mEnv(env), mString(s) {
            if (s == nullptr) {
                mChars = nullptr;
                mSize = 0;
            } else {
                mChars = mEnv->GetStringChars(mString, nullptr);
                mSize = mEnv->GetStringLength(mString);
            }
        }

        ~ScopedStringChars() {
            if (mChars != nullptr) {
                mEnv->ReleaseStringChars(mString, mChars);
            }
        }

        const jchar* get() const {
            return mChars;
        }

        size_t size() const {
            return mSize;
        }


    private:
        JNIEnv* const mEnv;
        const jstring mString;
        const jchar* mChars;
        size_t mSize;
    };

    struct FontMatcherDeleter {
        void operator()(AFontMatcher* matcher) { AFontMatcher_destroy(matcher); }
    };

    using FontMatcherUniquePtr = std::unique_ptr<AFontMatcher, FontMatcherDeleter>;

    class FontMatcher {
    public:
        FontMatcher() : mMatcher(AFontMatcher_create()) {}

        FontMatcher& setStyle(uint16_t weight, bool italic) {
            AFontMatcher_setStyle(mMatcher.get(), weight, italic);
            return *this;
        }

        FontMatcher& setLocales(const std::string& locales) {
            AFontMatcher_setLocales(mMatcher.get(), locales.c_str());
            return *this;
        }

        FontMatcher& setFamilyVariant(uint32_t familyVariant) {
            AFontMatcher_setFamilyVariant(mMatcher.get(), familyVariant);
            return *this;
        }

        std::pair<AFont*, uint32_t> match(const std::string familyName,
                                          const std::vector<uint16_t>& text) {
            uint32_t runLength;
            AFont* font = AFontMatcher_match(mMatcher.get(), familyName.c_str(), text.data(),
                                             text.size(), &runLength);
            return std::make_pair(font, runLength);
        }

    private:
        FontMatcherUniquePtr mMatcher;
    };


    jlong nOpenIterator(JNIEnv*, jclass) {
        return reinterpret_cast<jlong>(ASystemFontIterator_open());
    }

    void nCloseIterator(JNIEnv*, jclass, jlong ptr) {
        ASystemFontIterator_close(reinterpret_cast<ASystemFontIterator*>(ptr));
    }

    jlong nGetNext(JNIEnv*, jclass, jlong ptr) {
        return reinterpret_cast<jlong>(ASystemFontIterator_next(
                reinterpret_cast<ASystemFontIterator*>(ptr)));
    }

    void nCloseFont(JNIEnv*, jclass, jlong ptr) {
        return AFont_close(reinterpret_cast<AFont*>(ptr));
    }

    jstring nGetFilePath(JNIEnv* env, jclass, jlong ptr) {
        return env->NewStringUTF(AFont_getFontFilePath(reinterpret_cast<AFont*>(ptr)));
    }

    jint nGetWeight(JNIEnv*, jclass, jlong ptr) {
        return AFont_getWeight(reinterpret_cast<AFont*>(ptr));
    }

    jboolean nIsItalic(JNIEnv*, jclass, jlong ptr) {
        return AFont_isItalic(reinterpret_cast<AFont*>(ptr));
    }

    jstring nGetLocale(JNIEnv* env, jclass, jlong ptr) {
        return env->NewStringUTF(AFont_getLocale(reinterpret_cast<AFont*>(ptr)));
    }

    jint nGetCollectionIndex(JNIEnv*, jclass, jlong ptr) {
        return AFont_getCollectionIndex(reinterpret_cast<AFont*>(ptr));
    }

    jint nGetAxisCount(JNIEnv*, jclass, jlong ptr) {
        return AFont_getAxisCount(reinterpret_cast<AFont*>(ptr));
    }

    jint nGetAxisTag(JNIEnv*, jclass, jlong ptr, jint axisIndex) {
        return AFont_getAxisTag(reinterpret_cast<AFont*>(ptr), axisIndex);
    }

    jfloat nGetAxisValue(JNIEnv*, jclass, jlong ptr, jint axisIndex) {
        return AFont_getAxisValue(reinterpret_cast<AFont*>(ptr), axisIndex);
    }

    jlong nMatchFamilyStyleCharacter(JNIEnv* env, jclass, jstring familyName, jint weight,
                                     jboolean italic, jstring langTags, jint familyVariant,
                                     jstring text) {
        ScopedUtfChars familyNameChars(env, familyName);
        ScopedUtfChars langTagsChars(env, langTags);
        ScopedStringChars textChars(env, text);
        std::vector<uint16_t> utf16(textChars.get(), textChars.get() + textChars.size());
        return reinterpret_cast<jlong>(
                FontMatcher()
                        .setStyle(weight, italic)
                        .setLocales(langTagsChars.c_str())
                        .setFamilyVariant(familyVariant)
                        .match(familyNameChars.c_str(), utf16).first);
    }

    jint nMatchFamilyStyleCharacter_runLength(JNIEnv* env, jclass, jstring familyName, jint weight,
                                              jboolean italic, jstring langTags, jint familyVariant,
                                              jstring text) {
        ScopedUtfChars familyNameChars(env, familyName);
        ScopedUtfChars langTagsChars(env, langTags);
        ScopedStringChars textChars(env, text);
        std::vector<uint16_t> utf16(textChars.get(), textChars.get() + textChars.size());
        return FontMatcher()
                .setStyle(weight, italic)
                .setLocales(langTagsChars.c_str())
                .setFamilyVariant(familyVariant)
                .match(familyNameChars.c_str(), utf16).second;
    }

    const JNINativeMethod JNI_METHODS[] = {
         { "nOpenIterator", "()J", (void*) nOpenIterator },
         { "nCloseIterator", "(J)V", (void*) nCloseIterator },
         { "nNext", "(J)J", (void*) nGetNext },
         { "nCloseFont", "(J)V", (void*) nCloseFont },
         { "nGetFilePath", "(J)Ljava/lang/String;", (void*) nGetFilePath },
         { "nGetWeight", "(J)I", (void*) nGetWeight },
         { "nIsItalic", "(J)Z", (void*) nIsItalic },
         { "nGetLocale", "(J)Ljava/lang/String;", (void*) nGetLocale },
         { "nGetCollectionIndex", "(J)I", (void*) nGetCollectionIndex },
         { "nGetAxisCount", "(J)I", (void*) nGetAxisCount },
         { "nGetAxisTag", "(JI)I", (void*) nGetAxisTag },
         { "nGetAxisValue", "(JI)F", (void*) nGetAxisValue },
         { "nMatchFamilyStyleCharacter",
                 "(Ljava/lang/String;IZLjava/lang/String;ILjava/lang/String;)J",
                 (void*) nMatchFamilyStyleCharacter },
         { "nMatchFamilyStyleCharacter_runLength",
                 "(Ljava/lang/String;IZLjava/lang/String;ILjava/lang/String;)I",
                 (void*) nMatchFamilyStyleCharacter_runLength },
    };
}

int register_com_tuhailong_fontselector_SystemFontHelper(JNIEnv* env) {
    jclass clazz = env->FindClass("com/tuhailong/fontselector/SystemFontHelper");
    return env->RegisterNatives(clazz, JNI_METHODS, NELEM(JNI_METHODS));
}
