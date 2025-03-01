/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package mozilla.components.browser.state.engine.middleware

import kotlinx.coroutines.test.runTest
import mozilla.components.browser.state.action.BrowserAction
import mozilla.components.browser.state.action.InitAction
import mozilla.components.browser.state.action.TranslationsAction
import mozilla.components.browser.state.selector.findTab
import mozilla.components.browser.state.state.BrowserState
import mozilla.components.browser.state.state.TabSessionState
import mozilla.components.browser.state.state.TranslationsBrowserState
import mozilla.components.browser.state.state.TranslationsState
import mozilla.components.browser.state.state.createTab
import mozilla.components.browser.state.store.BrowserStore
import mozilla.components.concept.engine.Engine
import mozilla.components.concept.engine.EngineSession
import mozilla.components.concept.engine.translate.DetectedLanguages
import mozilla.components.concept.engine.translate.Language
import mozilla.components.concept.engine.translate.LanguageModel
import mozilla.components.concept.engine.translate.LanguageSetting
import mozilla.components.concept.engine.translate.TranslationDownloadSize
import mozilla.components.concept.engine.translate.TranslationEngineState
import mozilla.components.concept.engine.translate.TranslationError
import mozilla.components.concept.engine.translate.TranslationOperation
import mozilla.components.concept.engine.translate.TranslationPageSettingOperation
import mozilla.components.concept.engine.translate.TranslationPageSettings
import mozilla.components.concept.engine.translate.TranslationSupport
import mozilla.components.lib.state.MiddlewareContext
import mozilla.components.support.test.any
import mozilla.components.support.test.argumentCaptor
import mozilla.components.support.test.ext.joinBlocking
import mozilla.components.support.test.libstate.ext.waitUntilIdle
import mozilla.components.support.test.mock
import mozilla.components.support.test.rule.MainCoroutineRule
import mozilla.components.support.test.whenever
import org.junit.Before
import org.junit.Rule
import org.junit.Test
import org.mockito.ArgumentMatchers.anyBoolean
import org.mockito.Mockito.atLeastOnce
import org.mockito.Mockito.never
import org.mockito.Mockito.spy
import org.mockito.Mockito.verify

class TranslationsMiddlewareTest {

    @get:Rule
    val coroutinesTestRule = MainCoroutineRule()
    private val scope = coroutinesTestRule.scope
    private val engine: Engine = mock()
    private val engineSession: EngineSession = mock()
    private val tab: TabSessionState = spy(
        createTab(
            url = "https://www.firefox.com",
            title = "Firefox",
            id = "1",
            engineSession = engineSession,
        ),
    )
    private val translationsMiddleware = TranslationsMiddleware(engine = engine, scope = scope)
    private val tabs = spy(listOf(tab))
    private val state = spy(BrowserState(tabs = tabs))
    private val store = spy(BrowserStore(middleware = listOf(translationsMiddleware), initialState = state))
    private val context = mock<MiddlewareContext<BrowserState, BrowserAction>>()

    // Mock Variables
    private val mockFrom = Language(code = "es", localizedDisplayName = "Spanish")
    private val mockTo = Language(code = "en", localizedDisplayName = "English")
    private val mockSupportedLanguages = TranslationSupport(
        fromLanguages = listOf(mockFrom, mockTo),
        toLanguages = listOf(mockFrom, mockTo),
    )
    private val mockIsDownloaded = true
    private val mockSize: Long = 1234
    private val mockLanguage = Language(mockFrom.code, mockFrom.localizedDisplayName)
    private val mockLanguageModel = LanguageModel(mockLanguage, mockIsDownloaded, mockSize)
    private val mockLanguageModels = mutableListOf(mockLanguageModel)

    @Before
    fun setup() {
        whenever(context.store).thenReturn(store)
        whenever(context.state).thenReturn(state)
    }

    private fun waitForIdle() {
        scope.testScheduler.runCurrent()
        scope.testScheduler.advanceUntilIdle()
        coroutinesTestRule.testDispatcher.scheduler.advanceUntilIdle()
        store.waitUntilIdle()
    }

    /**
     * Use with tests that need a mock translations engine state and supported languages.
     */
    private fun setupMockState() {
        val mockDetectedLanguages = DetectedLanguages(
            documentLangTag = mockFrom.code,
            supportedDocumentLang = true,
            userPreferredLangTag = mockTo.code,
        )
        val mockSessionState = TranslationsState(
            translationEngineState = TranslationEngineState(mockDetectedLanguages),
        )
        whenever(store.state.findTab(tab.id)?.translationsState).thenReturn(mockSessionState)

        val mockBrowserState = TranslationsBrowserState(isEngineSupported = true, supportedLanguages = mockSupportedLanguages)
        whenever(store.state.translationEngine).thenReturn(mockBrowserState)
    }

    @Test
    fun `WHEN OperationRequestedAction is dispatched for FETCH_SUPPORTED_LANGUAGES AND succeeds THEN SetSupportedLanguagesAction is dispatched`() = runTest {
        // Initial Action
        val action =
            TranslationsAction.OperationRequestedAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_SUPPORTED_LANGUAGES,
            )

        translationsMiddleware.invoke(context = context, next = {}, action = action)

        // Verify results
        val languageCallback = argumentCaptor<((TranslationSupport) -> Unit)>()
        // Verifying at least once because `InitAction` also occurred
        verify(engine, atLeastOnce()).getSupportedTranslationLanguages(onSuccess = languageCallback.capture(), onError = any())
        val supportedLanguages = TranslationSupport(
            fromLanguages = listOf(Language("en", "English")),
            toLanguages = listOf(Language("en", "English")),
        )
        languageCallback.value.invoke(supportedLanguages)

        waitForIdle()

        verify(context.store, atLeastOnce()).dispatch(
            TranslationsAction.SetSupportedLanguagesAction(
                supportedLanguages = supportedLanguages,
            ),
        )

        verify(context.store, atLeastOnce()).dispatch(
            TranslationsAction.TranslateSuccessAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_SUPPORTED_LANGUAGES,
            ),
        )

        waitForIdle()
    }

    @Test
    fun `WHEN OperationRequestedAction is dispatched for FETCH_SUPPORTED_LANGUAGES AND fails THEN EngineExceptionAction is dispatched`() {
        // Initial Action
        val action =
            TranslationsAction.OperationRequestedAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_SUPPORTED_LANGUAGES,
            )

        translationsMiddleware.invoke(context = context, next = {}, action = action)

        // Verify results
        val errorCaptor = argumentCaptor<((Throwable) -> Unit)>()
        // Verifying at least once because `InitAction` also occurred
        verify(engine, atLeastOnce()).getSupportedTranslationLanguages(onSuccess = any(), onError = errorCaptor.capture())
        errorCaptor.value.invoke(Throwable())

        waitForIdle()

        verify(store, atLeastOnce()).dispatch(
            TranslationsAction.EngineExceptionAction(
                error = TranslationError.CouldNotLoadLanguagesError(any()),
            ),
        )

        verify(store, atLeastOnce()).dispatch(
            TranslationsAction.TranslateExceptionAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_SUPPORTED_LANGUAGES,
                translationError = TranslationError.CouldNotLoadLanguagesError(any()),
            ),
        )

        waitForIdle()
    }

    @Test
    fun `WHEN InitAction is dispatched THEN InitTranslationsBrowserState is also dispatched`() = runTest {
        // Send Action
        // Note: Will cause a double InitAction
        translationsMiddleware.invoke(context = context, next = {}, action = InitAction)
        waitForIdle()

        verify(store, atLeastOnce()).dispatch(
            TranslationsAction.InitTranslationsBrowserState,
        )
        waitForIdle()
    }

    @Test
    fun `WHEN InitTranslationsBrowserState is dispatched AND the engine is supported THEN SetSupportedLanguagesAction is also dispatched`() = runTest {
        // Send Action
        translationsMiddleware.invoke(context = context, next = {}, action = TranslationsAction.InitTranslationsBrowserState)

        // Set the engine to support
        val engineSupportedCallback = argumentCaptor<((Boolean) -> Unit)>()
        // At least once, since InitAction also will trigger this
        verify(engine, atLeastOnce()).isTranslationsEngineSupported(
            onSuccess = engineSupportedCallback.capture(),
            onError = any(),
        )
        engineSupportedCallback.value.invoke(true)

        // Verify results for language query
        val languageCallback = argumentCaptor<((TranslationSupport) -> Unit)>()
        verify(engine, atLeastOnce()).getSupportedTranslationLanguages(onSuccess = languageCallback.capture(), onError = any())
        val supportedLanguages = TranslationSupport(
            fromLanguages = listOf(Language("en", "English")),
            toLanguages = listOf(Language("en", "English")),
        )
        languageCallback.value.invoke(supportedLanguages)

        waitForIdle()

        // Verifying at least once
        verify(store, atLeastOnce()).dispatch(
            TranslationsAction.SetSupportedLanguagesAction(
                supportedLanguages = supportedLanguages,
            ),
        )

        waitForIdle()
    }

    @Test
    fun `WHEN InitTranslationsBrowserState is dispatched AND the engine is supported THEN SetLanguageSettingsAction is also dispatched`() = runTest {
        // Send Action
        translationsMiddleware.invoke(context = context, next = {}, action = TranslationsAction.InitTranslationsBrowserState)
        waitForIdle()

        // Set the engine to support
        val engineSupportedCallback = argumentCaptor<((Boolean) -> Unit)>()
        // At least once, since InitAction also will trigger this
        verify(engine, atLeastOnce()).isTranslationsEngineSupported(
            onSuccess = engineSupportedCallback.capture(),
            onError = any(),
        )
        engineSupportedCallback.value.invoke(true)

        // Check expectations
        val languageSettingsCallback = argumentCaptor<((Map<String, LanguageSetting>) -> Unit)>()
        verify(engine, atLeastOnce()).getLanguageSettings(
            onSuccess = languageSettingsCallback.capture(),
            onError = any(),
        )
        val mockLanguageSetting = mapOf("en" to LanguageSetting.OFFER)
        languageSettingsCallback.value.invoke(mockLanguageSetting)
        waitForIdle()

        verify(store, atLeastOnce()).dispatch(
            TranslationsAction.SetLanguageSettingsAction(
                languageSettings = mockLanguageSetting,
            ),
        )
        waitForIdle()
    }

    @Test
    fun `WHEN InitTranslationsBrowserState is dispatched AND an error occurs THEN TranslateExceptionAction is dispatched for language settings`() = runTest() {
        // Send Action
        translationsMiddleware.invoke(context = context, next = {}, action = TranslationsAction.InitTranslationsBrowserState)
        waitForIdle()

        // Set the engine to support
        val engineSupportedCallback = argumentCaptor<((Boolean) -> Unit)>()
        // At least once, since InitAction also will trigger this
        verify(engine, atLeastOnce()).isTranslationsEngineSupported(
            onSuccess = engineSupportedCallback.capture(),
            onError = any(),
        )
        engineSupportedCallback.value.invoke(true)

        // Check expectations
        val errorCallback = argumentCaptor<((Throwable) -> Unit)>()
        verify(engine, atLeastOnce()).getLanguageSettings(
            onSuccess = any(),
            onError = errorCallback.capture(),
        )
        errorCallback.value.invoke(Throwable())
        waitForIdle()

        verify(store, atLeastOnce()).dispatch(
            TranslationsAction.EngineExceptionAction(
                error = TranslationError.CouldNotLoadLanguageSettingsError(any()),
            ),
        )
        waitForIdle()
    }

    @Test
    fun `WHEN InitTranslationsBrowserState is dispatched AND the engine is supported THEN SetLanguageModelsAction is also dispatched`() = runTest {
        // Send Action
        translationsMiddleware.invoke(context = context, next = {}, action = TranslationsAction.InitTranslationsBrowserState)

        // Set the engine to support
        val engineSupportedCallback = argumentCaptor<((Boolean) -> Unit)>()
        // At least once, since InitAction also will trigger this
        verify(engine, atLeastOnce()).isTranslationsEngineSupported(
            onSuccess = engineSupportedCallback.capture(),
            onError = any(),
        )
        engineSupportedCallback.value.invoke(true)

        val languageCallback = argumentCaptor<((List<LanguageModel>) -> Unit)>()
        verify(engine, atLeastOnce()).getTranslationsModelDownloadStates(onSuccess = languageCallback.capture(), onError = any())
        languageCallback.value.invoke(mockLanguageModels)

        waitForIdle()

        // Verifying at least once
        verify(store, atLeastOnce()).dispatch(
            TranslationsAction.SetLanguageModelsAction(
                languageModels = mockLanguageModels,
            ),
        )
    }

    @Test
    fun `WHEN InitTranslationsBrowserState is dispatched AND has an issue with the engine THEN EngineExceptionAction is dispatched`() = runTest() {
        // Send Action
        // Note: Implicitly called once due to connection with InitAction
        translationsMiddleware.invoke(context = context, next = {}, action = TranslationsAction.InitTranslationsBrowserState)
        waitForIdle()

        // Check expectations
        val errorCallback = argumentCaptor<((Throwable) -> Unit)>()
        verify(engine, atLeastOnce()).isTranslationsEngineSupported(
            onSuccess = any(),
            onError = errorCallback.capture(),
        )
        errorCallback.value.invoke(IllegalStateException())
        waitForIdle()

        verify(store, atLeastOnce()).dispatch(
            TranslationsAction.EngineExceptionAction(
                error = TranslationError.UnknownEngineSupportError(any()),
            ),
        )
    }

    @Test
    fun `WHEN InitTranslationsBrowserState is dispatched AND the engine is not supported THEN SetSupportedLanguagesAction and SetLanguageModelsAction are NOT dispatched`() = runTest {
        // Send Action
        // Will invoke a double InitAction
        translationsMiddleware.invoke(context = context, next = {}, action = TranslationsAction.InitTranslationsBrowserState)

        // Set the engine to not support
        val engineNotSupportedCallback = argumentCaptor<((Boolean) -> Unit)>()
        verify(engine, atLeastOnce()).isTranslationsEngineSupported(
            onSuccess = engineNotSupportedCallback.capture(),
            onError = any(),
        )
        engineNotSupportedCallback.value.invoke(false)

        // Verify language query was never called
        verify(engine, never()).getSupportedTranslationLanguages(onSuccess = any(), onError = any())
        waitForIdle()
    }

    @Test
    fun `WHEN TranslateExpectedAction is dispatched THEN FetchTranslationDownloadSizeAction is also dispatched`() = runTest {
        // Set up the state of defaults on the engine.
        setupMockState()

        // Action
        translationsMiddleware.invoke(context = context, next = {}, action = TranslationsAction.TranslateExpectedAction(tab.id))

        waitForIdle()

        // Verifying at least once
        verify(store).dispatch(
            TranslationsAction.FetchTranslationDownloadSizeAction(
                tabId = tab.id,
                fromLanguage = mockFrom,
                toLanguage = mockTo,
            ),
        )

        waitForIdle()
    }

    @Test
    fun `WHEN TranslateExpectedAction is dispatched AND the defaults are NOT available THEN FetchTranslationDownloadSizeAction is NOT dispatched`() = runTest {
        // Note, no state is set on the engine, so no default values are available.
        // Action
        translationsMiddleware.invoke(context = context, next = {}, action = TranslationsAction.TranslateExpectedAction(tab.id))

        waitForIdle()

        // Verifying no dispatch
        verify(store, never()).dispatch(
            TranslationsAction.FetchTranslationDownloadSizeAction(
                tabId = tab.id,
                fromLanguage = mockFrom,
                toLanguage = mockTo,
            ),
        )

        // Verify language query was never called
        verify(engine, never()).getTranslationsModelDownloadStates(onSuccess = any(), onError = any())
        waitForIdle()
    }

    @Test
    fun `WHEN OperationRequestedAction is dispatched WITH FETCH_PAGE_SETTINGS AND fetching settings is successful THEN TranslationPageSettings is dispatched`() = runTest {
        // Setup
        setupMockState()

        val mockPageSettings = TranslationPageSettings(
            alwaysOfferPopup = true,
            alwaysTranslateLanguage = true,
            neverTranslateLanguage = false,
            neverTranslateSite = true,
        )

        whenever(engine.getTranslationsOfferPopup()).thenAnswer { mockPageSettings.alwaysOfferPopup }

        // Send Action
        val action =
            TranslationsAction.OperationRequestedAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_PAGE_SETTINGS,
            )
        translationsMiddleware.invoke(context, {}, action)
        waitForIdle()

        // Check Behavior
        // Popup always offer behavior
        verify(engine).getTranslationsOfferPopup()

        // Page language behavior
        val languageSettingCallback = argumentCaptor<((LanguageSetting) -> Unit)>()
        verify(engine).getLanguageSetting(
            languageCode = any(),
            onSuccess = languageSettingCallback.capture(),
            onError = any(),
        )
        val languageResponse = LanguageSetting.ALWAYS
        languageSettingCallback.value.invoke(languageResponse)

        // Never translate site behavior behavior
        val neverTranslateSiteCallback = argumentCaptor<((Boolean) -> Unit)>()
        verify(engineSession).getNeverTranslateSiteSetting(
            onResult = neverTranslateSiteCallback.capture(),
            onException = any(),
        )
        neverTranslateSiteCallback.value.invoke(mockPageSettings.neverTranslateSite!!)

        verify(store).dispatch(
            TranslationsAction.SetPageSettingsAction(
                tabId = tab.id,
                pageSettings = mockPageSettings,
            ),
        )
        waitForIdle()
    }

    @Test
    fun `WHEN OperationRequestedAction WITH FETCH_PAGE_SETTINGS AND fetching settings fails THEN TranslateExceptionAction is dispatched`() {
        // Setup
        setupMockState()
        whenever(engine.getTranslationsOfferPopup()).thenAnswer { false }

        // Send Action
        val action =
            TranslationsAction.OperationRequestedAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_PAGE_SETTINGS,
            )
        translationsMiddleware.invoke(context, {}, action)
        waitForIdle()

        // Check Behavior
        // Page language behavior
        val languageErrorCallback = argumentCaptor<((Throwable) -> Unit)>()
        verify(engine).getLanguageSetting(
            languageCode = any(),
            onSuccess = any(),
            onError = languageErrorCallback.capture(),
        )
        languageErrorCallback.value.invoke(Throwable())

        // Never translate site behavior behavior
        val neverTranslateSiteErrorCallback = argumentCaptor<((Throwable) -> Unit)>()
        verify(engineSession).getNeverTranslateSiteSetting(
            onResult = any(),
            onException = neverTranslateSiteErrorCallback.capture(),
        )
        neverTranslateSiteErrorCallback.value.invoke(Throwable())

        verify(store).dispatch(
            TranslationsAction.TranslateExceptionAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_PAGE_SETTINGS,
                translationError = TranslationError.CouldNotLoadPageSettingsError(any()),
            ),
        )

        waitForIdle()
    }

    @Test
    fun `WHEN UpdatePageSettingAction is dispatched WITH UPDATE_ALWAYS_TRANSLATE_LANGUAGE AND updating the setting is unsuccessful THEN OperationRequestedAction with FETCH_PAGE_SETTINGS is dispatched`() = runTest {
        // Setup
        setupMockState()
        val errorCallback = argumentCaptor<((Throwable) -> Unit)>()
        whenever(
            engine.setLanguageSetting(
                languageCode = any(),
                languageSetting = any(),
                onSuccess = any(),
                onError = errorCallback.capture(),
            ),
        ).thenAnswer { errorCallback.value.invoke(Throwable()) }

        // Send Action
        val action =
            TranslationsAction.UpdatePageSettingAction(
                tabId = tab.id,
                operation = TranslationPageSettingOperation.UPDATE_ALWAYS_TRANSLATE_LANGUAGE,
                setting = true,
            )
        translationsMiddleware.invoke(context, {}, action)
        waitForIdle()

        // Verify Dispatch
        verify(store).dispatch(
            TranslationsAction.OperationRequestedAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_PAGE_SETTINGS,
            ),
        )
    }

    @Test
    fun `WHEN an Operation to FETCH_AUTOMATIC_LANGUAGE_SETTINGS is dispatched THEN SetLanguageSettingsAction is dispatched`() = runTest {
        // Send Action
        val action =
            TranslationsAction.OperationRequestedAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_AUTOMATIC_LANGUAGE_SETTINGS,
            )
        translationsMiddleware.invoke(context = context, next = {}, action = action)
        waitForIdle()

        // Check expectations
        val languageSettingsCallback = argumentCaptor<((Map<String, LanguageSetting>) -> Unit)>()
        // Checking atLeastOnce, because InitAction is also implicitly called earlier
        verify(engine, atLeastOnce()).getLanguageSettings(
            onSuccess = languageSettingsCallback.capture(),
            onError = any(),
        )
        val mockLanguageSetting = mapOf("en" to LanguageSetting.OFFER)
        languageSettingsCallback.value.invoke(mockLanguageSetting)
        waitForIdle()

        verify(store, atLeastOnce()).dispatch(
            TranslationsAction.SetLanguageSettingsAction(
                languageSettings = mockLanguageSetting,
            ),
        )
        waitForIdle()
    }

    @Test
    fun `WHEN an Operation to UpdatePageSettings for UPDATE_ALWAYS_TRANSLATE_LANGUAGE is dispatched THEN SetLanguageSettingsAction is dispatched`() = runTest {
        // Page settings needs additional setup
        setupMockState()
        val pageSettingCallback = argumentCaptor<(() -> Unit)>()
        whenever(
            engine.setLanguageSetting(
                languageCode = any(),
                languageSetting = any(),
                onSuccess = pageSettingCallback.capture(),
                onError = any(),
            ),
        ).thenAnswer { pageSettingCallback.value.invoke() }

        // Send Action
        val action =
            TranslationsAction.UpdatePageSettingAction(
                tabId = tab.id,
                operation = TranslationPageSettingOperation.UPDATE_ALWAYS_TRANSLATE_LANGUAGE,
                setting = true,
            )
        translationsMiddleware.invoke(context = context, next = {}, action = action)
        waitForIdle()

        // Check expectations
        val languageSettingsCallback = argumentCaptor<((Map<String, LanguageSetting>) -> Unit)>()
        verify(engine).getLanguageSettings(
            onSuccess = languageSettingsCallback.capture(),
            onError = any(),
        )
        waitForIdle()
    }

    @Test
    fun `WHEN an Operation to FETCH_AUTOMATIC_LANGUAGE_SETTINGS has an error THEN EngineExceptionAction and TranslateExceptionAction are dispatched for language setting`() = runTest() {
        // Send Action
        val action =
            TranslationsAction.OperationRequestedAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_AUTOMATIC_LANGUAGE_SETTINGS,
            )
        translationsMiddleware.invoke(context = context, next = {}, action = action)
        waitForIdle()

        // Check expectations
        val errorCallback = argumentCaptor<((Throwable) -> Unit)>()
        verify(engine, atLeastOnce()).getLanguageSettings(
            onSuccess = any(),
            onError = errorCallback.capture(),
        )
        errorCallback.value.invoke(Throwable())
        waitForIdle()

        verify(store, atLeastOnce()).dispatch(
            TranslationsAction.EngineExceptionAction(
                error = TranslationError.CouldNotLoadLanguageSettingsError(any()),
            ),
        )

        verify(store, atLeastOnce()).dispatch(
            TranslationsAction.TranslateExceptionAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_AUTOMATIC_LANGUAGE_SETTINGS,
                translationError = TranslationError.CouldNotLoadLanguageSettingsError(any()),
            ),
        )
        waitForIdle()
    }

    @Test
    fun `WHEN UpdatePageSettingAction is dispatched WITH UPDATE_NEVER_TRANSLATE_LANGUAGE AND updating the setting is unsuccessful THEN OperationRequestedAction with FETCH_PAGE_SETTINGS is dispatched`() = runTest {
        // Setup
        setupMockState()
        val errorCallback = argumentCaptor<((Throwable) -> Unit)>()
        whenever(
            engine.setLanguageSetting(
                languageCode = any(),
                languageSetting = any(),
                onSuccess = any(),
                onError = errorCallback.capture(),
            ),
        )
            .thenAnswer { errorCallback.value.invoke(Throwable()) }

        // Send Action
        val action =
            TranslationsAction.UpdatePageSettingAction(
                tabId = tab.id,
                operation = TranslationPageSettingOperation.UPDATE_NEVER_TRANSLATE_LANGUAGE,
                setting = true,
            )
        translationsMiddleware.invoke(context, {}, action)
        waitForIdle()

        // Verify Dispatch
        verify(store).dispatch(
            TranslationsAction.OperationRequestedAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_PAGE_SETTINGS,
            ),
        )
        waitForIdle()
    }

    @Test
    fun `WHEN UpdatePageSettingAction is dispatched WITH UPDATE_NEVER_TRANSLATE_SITE AND updating the setting is unsuccessful THEN OperationRequestedAction with FETCH_PAGE_SETTINGS is dispatched`() = runTest {
        // Setup
        setupMockState()
        val errorCallback = argumentCaptor<((Throwable) -> Unit)>()
        whenever(
            engineSession.setNeverTranslateSiteSetting(
                setting = anyBoolean(),
                onResult = any(),
                onException = errorCallback.capture(),
            ),
        )
            .thenAnswer { errorCallback.value.invoke(Throwable()) }

        // Send Action
        val action =
            TranslationsAction.UpdatePageSettingAction(
                tabId = tab.id,
                operation = TranslationPageSettingOperation.UPDATE_NEVER_TRANSLATE_SITE,
                setting = true,
            )
        translationsMiddleware.invoke(context, {}, action)
        waitForIdle()

        // Verify Dispatch
        verify(store).dispatch(
            TranslationsAction.OperationRequestedAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_PAGE_SETTINGS,
            ),
        )
        waitForIdle()
    }

    @Test
    fun `WHEN OperationRequestedAction is dispatched to fetch never translate sites AND succeeds THEN SetNeverTranslateSitesAction is dispatched`() = runTest {
        val neverTranslateSites = listOf("google.com")
        val sitesCallback = argumentCaptor<((List<String>) -> Unit)>()
        val action =
            TranslationsAction.OperationRequestedAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_NEVER_TRANSLATE_SITES,
            )
        translationsMiddleware.invoke(context, {}, action)
        verify(engine).getNeverTranslateSiteList(onSuccess = sitesCallback.capture(), onError = any())
        sitesCallback.value.invoke(neverTranslateSites)

        verify(context.store).dispatch(
            TranslationsAction.SetNeverTranslateSitesAction(
                tabId = tab.id,
                neverTranslateSites = neverTranslateSites,
            ),
        )

        waitForIdle()
    }

    @Test
    fun `WHEN OperationRequestedAction is dispatched to fetch never translate sites AND fails THEN TranslateExceptionAction is dispatched`() = runTest {
        store.dispatch(
            TranslationsAction.OperationRequestedAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_NEVER_TRANSLATE_SITES,
            ),
        ).joinBlocking()
        waitForIdle()

        verify(store).dispatch(
            TranslationsAction.TranslateExceptionAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_NEVER_TRANSLATE_SITES,
                translationError = TranslationError.CouldNotLoadNeverTranslateSites(any()),
            ),
        )
        waitForIdle()
    }

    @Test
    fun `WHEN FetchTranslationDownloadSize is requested AND succeeds THEN SetTranslationDownloadSize is dispatched`() = runTest {
        val translationSize = TranslationDownloadSize(
            fromLanguage = Language("en", "English"),
            toLanguage = Language("fr", "French"),
            size = 10000L,
            error = null,
        )

        val action =
            TranslationsAction.FetchTranslationDownloadSizeAction(
                tabId = tab.id,
                fromLanguage = translationSize.fromLanguage,
                toLanguage = translationSize.toLanguage,
            )
        translationsMiddleware.invoke(context = context, next = {}, action = action)

        val sizeCaptor = argumentCaptor<((Long) -> Unit)>()
        verify(engine).getTranslationsPairDownloadSize(
            fromLanguage = any(),
            toLanguage = any(),
            onSuccess = sizeCaptor.capture(),
            onError = any(),
        )
        sizeCaptor.value.invoke(translationSize.size!!)

        verify(context.store).dispatch(
            TranslationsAction.SetTranslationDownloadSizeAction(
                tabId = tab.id,
                translationSize = translationSize,
            ),
        )

        waitForIdle()
    }

    @Test
    fun `WHEN FetchTranslationDownloadSize is requested AND fails THEN SetTranslationDownloadSize is dispatched`() = runTest {
        val action =
            TranslationsAction.FetchTranslationDownloadSizeAction(
                tabId = tab.id,
                fromLanguage = Language("en", "English"),
                toLanguage = Language("fr", "French"),
            )
        translationsMiddleware.invoke(context = context, next = {}, action = action)

        val errorCaptor = argumentCaptor<((Throwable) -> Unit)>()
        verify(engine).getTranslationsPairDownloadSize(
            fromLanguage = any(),
            toLanguage = any(),
            onSuccess = any(),
            onError = errorCaptor.capture(),
        )
        errorCaptor.value.invoke(TranslationError.CouldNotDetermineDownloadSizeError(cause = null))

        verify(context.store).dispatch(
            TranslationsAction.SetTranslationDownloadSizeAction(
                tabId = tab.id,
                translationSize = TranslationDownloadSize(
                    fromLanguage = Language("en", "English"),
                    toLanguage = Language("fr", "French"),
                    size = null,
                    error = any(),
                ),
            ),
        )

        waitForIdle()
    }

    @Test
    fun `WHEN RemoveNeverTranslateSiteAction is dispatched AND removing is unsuccessful THEN FETCH_NEVER_TRANSLATE_SITES is dispatched`() = runTest {
        val errorCallback = argumentCaptor<((Throwable) -> Unit)>()
        whenever(
            engine.setNeverTranslateSpecifiedSite(
                origin = any(),
                setting = anyBoolean(),
                onSuccess = any(),
                onError = errorCallback.capture(),
            ),
        ).thenAnswer { errorCallback.value.invoke(Throwable()) }

        val action =
            TranslationsAction.RemoveNeverTranslateSiteAction(
                tabId = tab.id,
                origin = "google.com",
            )
        translationsMiddleware.invoke(context, {}, action)
        waitForIdle()

        // Verify Dispatch
        verify(store).dispatch(
            TranslationsAction.OperationRequestedAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_NEVER_TRANSLATE_SITES,
            ),
        )
        waitForIdle()
    }

    @Test
    fun `WHEN RemoveNeverTranslateSiteAction is dispatched AND removing is successful THEN FETCH_PAGE_SETTINGS is dispatched`() = runTest {
        val sitesCallback = argumentCaptor<(() -> Unit)>()
        val action =
            TranslationsAction.RemoveNeverTranslateSiteAction(
                tabId = tab.id,
                origin = "google.com",
            )
        translationsMiddleware.invoke(context, {}, action)
        verify(engine).setNeverTranslateSpecifiedSite(
            origin = any(),
            setting = anyBoolean(),
            onSuccess = sitesCallback.capture(),
            onError = any(),
        )
        sitesCallback.value.invoke()
        waitForIdle()

        // Verify Dispatch
        verify(store).dispatch(
            TranslationsAction.OperationRequestedAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_PAGE_SETTINGS,
            ),
        )
        waitForIdle()
    }

    @Test
    fun `WHEN OperationRequestedAction is dispatched to FETCH_LANGUAGE_MODELS AND succeeds THEN SetLanguageModelsAction is dispatched`() = runTest {
        val languageCallback = argumentCaptor<((List<LanguageModel>) -> Unit)>()

        // Initial Action
        val action =
            TranslationsAction.OperationRequestedAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_LANGUAGE_MODELS,
            )
        translationsMiddleware.invoke(context, {}, action)

        // Verify results
        verify(engine, atLeastOnce()).getTranslationsModelDownloadStates(onSuccess = languageCallback.capture(), onError = any())
        languageCallback.value.invoke(mockLanguageModels)

        verify(context.store, atLeastOnce()).dispatch(
            TranslationsAction.SetLanguageModelsAction(
                languageModels = mockLanguageModels,
            ),
        )

        waitForIdle()
    }

    @Test
    fun `WHEN OperationRequestedAction is dispatched to FETCH_LANGUAGE_MODELS AND fails THEN TranslateExceptionAction is dispatched`() = runTest {
        val errorCallback = argumentCaptor<((Throwable) -> Unit)>()
        whenever(
            engine.getTranslationsModelDownloadStates(
                onSuccess = any(),
                onError = errorCallback.capture(),
            ),
        ).thenAnswer { errorCallback.value.invoke(Throwable()) }

        val action =
            TranslationsAction.OperationRequestedAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_LANGUAGE_MODELS,
            )
        translationsMiddleware.invoke(context, {}, action)
        waitForIdle()

        // Verify Dispatch
        verify(store, atLeastOnce()).dispatch(
            TranslationsAction.EngineExceptionAction(
                error = TranslationError.ModelCouldNotRetrieveError(any()),
            ),
        )

        verify(store, atLeastOnce()).dispatch(
            TranslationsAction.TranslateExceptionAction(
                tabId = tab.id,
                operation = TranslationOperation.FETCH_LANGUAGE_MODELS,
                translationError = TranslationError.ModelCouldNotRetrieveError(any()),
            ),
        )

        waitForIdle()
    }
}
