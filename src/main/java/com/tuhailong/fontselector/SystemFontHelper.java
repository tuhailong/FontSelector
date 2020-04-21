package com.tuhailong.fontselector;

import android.graphics.fonts.Font;
import android.graphics.fonts.FontStyle;
import android.graphics.fonts.FontVariationAxis;
import android.util.Pair;

import java.io.File;
import java.io.IOException;
import java.util.HashSet;
import java.util.Set;

public class SystemFontHelper {
    static {
        System.loadLibrary("font_selector");
    }

    private static String tagToStr(int tag) {
        char[] buf = new char[4];
        buf[0] = (char) ((tag >> 24) & 0xFF);
        buf[1] = (char) ((tag >> 16) & 0xFF);
        buf[2] = (char) ((tag >> 8) & 0xFF);
        buf[3] = (char) (tag & 0xFF);
        return String.valueOf(buf);
    }

    public static Set<Font> getAvailableFonts() {
        long iterPtr = nOpenIterator();
        HashSet<Font> nativeFonts = new HashSet<>();
        try {
            for (long fontPtr = nNext(iterPtr); fontPtr != 0; fontPtr = nNext(iterPtr)) {
                try {
                    FontVariationAxis[] axes = new FontVariationAxis[nGetAxisCount(fontPtr)];
                    for (int i = 0; i < axes.length; ++i) {
                        axes[i] = new FontVariationAxis(
                                tagToStr(nGetAxisTag(fontPtr, i)), nGetAxisValue(fontPtr, i));
                    }
                    nativeFonts.add(new Font.Builder(new File(nGetFilePath(fontPtr)))
                            .setWeight(nGetWeight(fontPtr))
                            .setSlant(nIsItalic(fontPtr)
                                    ?  FontStyle.FONT_SLANT_ITALIC : FontStyle.FONT_SLANT_UPRIGHT)
                            .setTtcIndex(nGetCollectionIndex(fontPtr))
                            .setFontVariationSettings(axes)
                            .build());
                } catch (IOException e) {
                    throw new RuntimeException(e);
                } finally {
                    nCloseFont(fontPtr);
                }
            }
        } finally {
            nCloseIterator(iterPtr);
        }
        return nativeFonts;
    }

    public static Pair<File, Integer> matchFamilyStyleCharacter(String familyName, int weight,
                                                                boolean italic, String languageTags, int familyVariant, String text) {
        final long fontPtr = nMatchFamilyStyleCharacter(familyName, weight, italic, languageTags,
                familyVariant, text);
        final int runLength = nMatchFamilyStyleCharacter_runLength(familyName, weight, italic,
                languageTags, familyVariant, text);
        try {
            return new Pair<>(new File(nGetFilePath(fontPtr)), runLength);
        } finally {
            nCloseFont(fontPtr);
        }
    }

    private static native long nOpenIterator();
    private static native void nCloseIterator(long ptr);
    private static native long nNext(long ptr);
    private static native void nCloseFont(long ptr);
    private static native String nGetFilePath(long ptr);
    private static native int nGetWeight(long ptr);
    private static native boolean nIsItalic(long ptr);
    private static native String nGetLocale(long ptr);
    private static native int nGetCollectionIndex(long ptr);
    private static native int nGetAxisCount(long ptr);
    private static native int nGetAxisTag(long ptr, int index);
    private static native float nGetAxisValue(long ptr, int index);
    private static native long nMatchFamilyStyleCharacter(String familyName, int weight,
            boolean italic, String languageTags, int familyVariant, String text);
    private static native int nMatchFamilyStyleCharacter_runLength(String familyName, int weight,
            boolean italic, String languageTags, int familyVariant, String text);
}
