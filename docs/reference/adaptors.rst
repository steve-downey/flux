
Adaptors
========

.. namespace:: flux

``adjacent``
^^^^^^^^^^^^

.. function::
   template <distance_t N> \
      requires (N > 0) \
   auto adjacent(multipass_sequence auto seq) -> multipass_sequence auto

   Given a compile-time size :var:`N` and a multipass sequence :var:`seq`, returns a new sequence which yields sliding windows of size :var:`N` as an :var:`N`-tuple of elements of :var:`seq`. If `seq` has fewer than :var:`N` elements, the adapted sequence will be empty.

   The returned sequence is always a :concept:`multipass_sequence`. It is also
    * bidirectional when :var:`seq` is bidirectional
    * random-access when :var:`seq` is random-access
    * sized when :var:`seq` is sized
    * bounded when :var:`seq` is *both* bounded and bidirectional

   The :func:`slide()` adaptor is similar to :func:`adjacent()`, but takes its window size as a run-time rather than a compile-time parameter, and returns length-:any:`n` subsequences rather than tuples.

   Equivalent to::

       zip(seq, drop(seq, 1), drop(seq, 2), ..., drop(seq, N-1));

   :tparam N: The size of the sliding window. Must be greater than zero.

   :param seq: A multipass sequence.

   :returns: A sequence adaptor whose element type is a :expr:`std::tuple` of size :var:`N`, or a :expr:`std::pair` if :expr:`N == 2`.

   :example:

    ..  literalinclude:: ../../example/docs/adjacent.cpp
        :language: cpp
        :dedent:
        :lines: 15-26

   :see also:

        * `std::views::adjacent <https://en.cppreference.com/w/cpp/ranges/adjacent_view>`_ (C++23)
        * :func:`flux::adjacent_map`
        * :func:`flux::pairwise`
        * :func:`flux::slide`

``adjacent_filter``
^^^^^^^^^^^^^^^^^^^

..  function::
    template <multipass_sequence Seq, typename Pred> \
        requires std::predicate<Pred&, element_t<Seq>, element_t<Seq>> \
    auto adjacent_filter(Seq seq, Pred pred) -> multipass_sequence auto;

    Applies the given binary predicate :var:`pred` to each pair of adjacent elements of :var:`seq`. If the predicate returns ``false``, the second element of the pair does not appear in the resulting sequence. The first element of :var:`seq` is always included in the output.

    The resulting sequence is always multipass; it is also a :concept:`bidirectional_sequence` if :var:`Seq` is bidirectional, and a :concept:`bounded_sequence` if :var:`Seq` is bounded.

    A common use for :func:`adjacent_filter` is to remove adjacent equal elements from a sequence, which can be achieved by passing :expr:`std::not_equal_to{}` as the predicate. The :func:`dedup` function is a handy alias for :expr:`adjacent_filter(not_equal_to{})`.

    :param seq: A multipass sequence
    :param pred: A binary predicate to compare sequence elements

    :returns: The filtered sequence

    :example:

    ..  literalinclude:: ../../example/docs/adjacent_filter.cpp
        :language: cpp
        :dedent:
        :lines: 15-34

    :see also:
       * :func:`flux::dedup`
       * :func:`flux::filter`


``adjacent_map``
^^^^^^^^^^^^^^^^

.. function::
   template <distance_t N> \
   auto adjacent_map(multipass_sequence auto seq, auto func) -> multipass_sequence auto;

``cache_last``
^^^^^^^^^^^^^^

..  function::
    template <sequence Seq> \
        requires see_below \
    auto cache_last(Seq seq) -> sequence auto;

``cartesian_product``
^^^^^^^^^^^^^^^^^^^^^

..  function::
    auto cartesian_product(sequence auto seq0, multipass_sequence auto... seqs) -> sequence auto;

``cartesian_product_map``
^^^^^^^^^^^^^^^^^^^^^^^^^

..  function::
    template <typename Func, sequence Seq0, multipass_sequence... Seqs> \
    requires std::regular_invocable<Func&, element_t<Seq0>, element_t<Seqs>...> \
    auto cartesian_product_with(Func func, Seq0 seq0, Seqs... seqs) -> sequence auto;

``chain``
^^^^^^^^^

..  function::
    template <sequence Seq0, sequence... Seqs> \
        requires see_below \
    auto chain(Seq0 seq0, Seqs... seqs) -> sequence auto;

``chunk``
^^^^^^^^^

..  function::
    auto chunk(sequence auto seq, std::integral auto chunk_sz) -> sequence auto;

``chunk_by``
^^^^^^^^^^^^

..  function::
    template <multipass_sequence Seq, typename Pred> \
        requires std::predicate<Pred, element_t<Seq>, element_t<Seq>> \
    auto chunk_by(Seq seq, Pred pred) -> multipass_sequence auto;

``cursors``
^^^^^^^^^^^

..  function::
    auto cursors(multipass_sequence auto seq) -> multipass_sequence auto;

    Given a sequence :var:`seq`, :expr:`cursors(seq)` returns a new sequence whose elements are the cursors of the original sequence. The :func:`cursors` sequence retains all the capabilities of the source sequence (bidirectional, random access, sized etc), up to :concept:`contiguous_sequence`.

    This is basically a passthrough adaptor, except that :expr:`read_at(seq, cur)` returns a copy of :var:`cur`.

    :param seq: A multipass sequence

    :returns: A sequence whose elements are the cursors of :var:`seq`

    :example:

    ..  literalinclude:: ../../example/docs/cursors.cpp
        :language: cpp
        :dedent:
        :lines: 16-29


``cycle``
^^^^^^^^^

..  function::
    template <sequence Seq> \
        requires multipass_sequence<Seq> || infinite_sequence<Seq> \
    auto cycle(Seq seq) -> infinite_sequence auto;

..  function::
    template <multipass_sequence Seq>\
    auto cycle(Seq seq, std::integral auto count) \
        -> multipass_sequence auto;

    Repeats the elements of :var:`seq` endlessly (for the first overload) or :var:`count` times (for the second overload).

    For the first overload, if :var:`Seq` is already an :concept:`infinite_sequence`, it is passed through unchanged.

    Otherwise, both overloads require a :concept:`multipass_sequence`, and the output is always a :concept:`multipass_sequence`. The adapted sequence is also a :concept:`bidirectional_sequence` when :var:`Seq` is both bidirectional and bounded, and a :concept:`random_access_sequence` when :var:`Seq` is random-access and bounded.

    For the second overload, the returned sequence is additionally always a :concept:`bounded_sequence` (even if :var:`Seq` is not), and a :concept:`sized_sequence` when the source sequence is sized.

    To avoid "spooky action at a distance" (where mutating :expr:`s[n]` would change the value of some other :expr:`s[m]`) :func:`cycle` provides only immutable access to the elements of :var:`seq`: that is, it behaves as if :var:`seq` were first passed through :func:`read_only`.

    .. caution::

        In order to provide random-access functionality, cursors for cycled sequences keep a :type:`size_t` count of how many times they have looped round. For very long-running programs using the infinite version of :func:`cycle` it may be possible to overflow this counter. (Assuming 1000 iterations per second, this would take approximately 49 days on a machine with a 32-bit :type:`size_t`, or around 500 million years on a 64-bit machine.)

        While this won't cause undefined behaviour, it's possible to encounter incorrect results or runtime errors when using the random-access functions on cursors which have overflowed.

    :param seq: A sequence to cycle through

    :param count: The number of times to loop through the sequence before terminating. If not supplied, the sequence will be repeated endlessly.

    :returns: An adapted sequence which repeatedly loops through the elements of :var:`seq`.

    :example:

    ..  literalinclude:: ../../example/docs/cycle.cpp
        :language: cpp
        :dedent:
        :lines: 14-37

    :see also:

``dedup``
^^^^^^^^^

..  function::
    template <multipass_sequence Seq> \
        requires std::equality_comparable<element_t<Seq>> \
    auto dedup(Seq seq) -> multipass_sequence auto;

    An alias for :expr:`adjacent_filter(seq, std::ranges::not_equal_to{})`. This can be used to remove adjacent elements from a sequence.

    :see also:
        * :func:`flux::adjacent_filter`

``drop``
^^^^^^^^

..  function::
    auto drop(sequence auto seq, std::integral auto count) -> sequence auto;

    Given a sequence :var:`seq` and a non-negative integral value :var:`count`, returns a new sequence which skips the first :var:`count` elements of :var:`seq`.

    The returned sequence has the same capabilities as :var:seq. If :var:`seq` has fewer than :var:`count` elements, the returned sequence is empty.

    :param seq: A sequence.
    :param count: A non-negative integral value indicating the number of elements to be skipped.

    :returns: A sequence adaptor that yields the remaining elements of :var:`seq`, with the first :var:`count` elements skipped.

    :example:

    ..  literalinclude:: ../../example/docs/drop.cpp
        :language: cpp
        :dedent:
        :lines: 14-19

    :see also:

        * `std::views::drop <https://en.cppreference.com/w/cpp/ranges/drop_view>`_ (C++20)
        * :func:`flux::take`

``drop_while``
^^^^^^^^^^^^^^

..  function::
    template <sequence Seq, typename Pred> \
        requires std::predicate<Pred&, element_t<Seq>> \
    auto drop_while(Seq seq, Pred pred) -> sequence auto;

``filter``
^^^^^^^^^^

..  function::
    template <sequence Seq, typename Pred> \
        requires std::predicate<Pred, element_t<Seq>&> \
    auto filter(Seq seq, Pred pred) -> sequence auto;

``flatten``
^^^^^^^^^^^

..  function::
    template <sequence Seq> \
        requires sequence<element_t<Seq>> \
    auto flatten(Seq seq) -> sequence auto;

``map``
^^^^^^^

.. function::
   template <sequence Seq, std::copy_constructable Func> \
   auto map(Seq seq, Func func) -> sequence auto;

``mask``
^^^^^^^^

..  function::
    template <sequence Seq, sequence Mask> \
        requires boolean_testable<element_t<Mask>> \
    auto mask(Seq seq, Mask where) -> sequence auto;

    Given a sequence of values and a sequence of booleans, :func:`mask` yields those elements of :var:`seq` for which the corresponding element of :var:`where` evaluates to :expr:`true`. Iteration is complete when either of the two input sequences is exhausted.

    The returned sequence models the lowest common category of the two input sequences, up to :concept:`bidirectional_sequence`. It is also a :concept:`bounded_sequence` and a :concept:`sized_sequence` when both inputs model these concepts.

    :param seq: A sequence of values
    :param where: A sequence whose element type is convertible to :expr:`bool`

    :returns: An adapted sequence whose elements are the elements of :var:`seq`, filtered by the corresponding elements of :var:`where`

    :example:

    ..  literalinclude:: ../../example/docs/mask.cpp
        :language: cpp
        :dedent:
        :lines: 16-30

    :see also:
        * :func:`flux::filter`

``pairwise``
^^^^^^^^^^^^

..  function::
    auto pairwise(multipass_sequence auto seq) -> multipass_sequence auto;

    Returns an adaptor which yields pairs of elements of :var:`seq`. It is an alias for :func:`adjacent\<2>`.

    :param seq: A multipass sequence.

    :returns: A multipass sequence yielding pairs of elements of :var:`seq`.


``pairwise_map``
^^^^^^^^^^^^^^^^

..  function::
    template <multipass_sequence Seq, typename Func> \
    requires std::regular_invocable<Func&, element_t<Seq>, element_t<Seq>> && \
             can_reference<std::invoke_result_t<Func&, element_t<Seq>, element_t<Seq>>> \
    auto pairwise_map(Seq seq, Func func) -> multipass_sequence auto;

``prescan``
^^^^^^^^^^^

..  function::
    template <sequence Seq, typename Func, std::movable Init> \
        requires foldable<Seq, Func, Init> \
    auto prescan(Seq seq, Func func, Init init) -> sequence auto;

    Returns a stateful sequence adaptor which yields "partial folds" using the binary function :var:`func`.

    First, this adaptor initialises an internal variable :var:`state` to :var:`init` and yields a read-only reference to this state. Then, for each successive element :var:`elem` of the underlying sequence, it sets::

        state = func(std::move(state), std::forward(elem));

    and yields a read-only reference to the new state.

    The final value yielded by this adaptor is the same as :expr:`fold(seq, func, init)`.

    Because this adaptor needs to maintain internal state, it is only ever single-pass. However it is a :concept:`bounded_sequence` when the underlying sequence is bounded and a :concept:`sized_sequence` when the underlying sequence is sized.

    Unlike :func:`scan`, this function performs an *exclusive scan*, that is, the Nth element of the adapted sequence does not include the Nth element of the underlying sequence. The adaptor returned by :func:`prescan` always yields at least one element -- the initial value -- followed by the elements that would be yielded by the :func:`scan` adaptor.

    :param seq: A sequence to adapt
    :param func: A binary callable of the form :expr:`R(R, element_t<Seq>)`, where :type:`R` is constructible from :var:`Init`
    :param init: The initial value for the scan

    :returns: A sequence adaptor which performs an exclusive scan of the elements of :var:`seq` using :var:`func`.

    :example:

    ..  literalinclude:: ../../example/docs/prescan.cpp
        :language: cpp
        :dedent:
        :lines: 13-20

    :see also:
        * `std::exclusive_scan() <https://en.cppreference.com/w/cpp/algorithm/exclusive_scan>`_
        * :func:`flux::scan`
        * :func:`flux::fold`

``read_only``
^^^^^^^^^^^^^

..  function::
    template <sequence Seq> \
    auto read_only(Seq seq) -> read_only_sequence auto;

    Returns an adapted sequence which prevents direct modification of the elements of :var:`seq`. The returned sequence retains the capabilities of the source sequence, all the way up to :concept:`contiguous_sequence`.

    If :var:`Seq` is already a :concept:`read_only_sequence`, then it is returned unchanged. Otherwise, :func:`read_only` is equivalent to::

        map(seq, [](auto&& elem) -> const_element_t<Seq> {
            return static_cast<const_element_t<Seq>>(std::forward(elem));
        });

    except that the returned sequence will be a :concept:`contiguous_sequence` if the source sequence models that concept. In this case, the pointer returned from :func:`data` will have type :expr:`value_t<Seq> const*`.

    :param seq: A sequence

    :returns: An adapted sequence which provides read-only access to the elements of :var:`seq`

    :example:

    ..  literalinclude:: ../../example/docs/read_only.cpp
        :language: cpp
        :dedent:
        :lines: 12-34

    :see also:
        * `std::views::as_const() <https://en.cppreference.com/w/cpp/ranges/as_const_view>`_ (C++23)
        * :func:`flux::map`

``reverse``
^^^^^^^^^^^

..  function::
    template <bidirectional_sequence Seq> \
        requires bounded_sequence<Seq> \
    auto reverse(Seq seq) -> bidirectional_sequence auto;

``scan``
^^^^^^^^

..  function::
    template <sequence Seq, typename Func, std::movable Init = value_t<Seq>> \
        requires foldable<Seq, Func, Init> \
    auto scan(Seq seq, Func func, Init init = {}) -> sequence auto;

    Returns a stateful sequence adaptor which yields "partial folds" using the binary function :var:`func`.

    First, this adaptor initialises an internal variable :var:`state` to :var:`init`. Then, for each successive element :var:`elem` of the underlying sequence, it sets::

        state = func(std::move(state), std::forward(elem));

    and yields a read-only reference to the new state.

    The final value yielded by this adaptor is the same as :expr:`fold(seq, func, init)`.

    Because this adaptor needs to maintain internal state, it is only ever single-pass. However it is a :concept:`bounded_sequence` when the underlying sequence is bounded and a :concept:`sized_sequence` when the underlying sequence is sized.

    Unlike :func:`prescan`, this function performs an *inclusive scan*, that is, the Nth element of the adapted sequence includes the Nth element of the underlying sequence. The adapted sequence always yields the same number of elements as the underlying sequence.

    :param seq: A sequence to adapt
    :param func: A binary callable of the form :expr:`R(R, element_t<Seq>)`, where :type:`R` is constructible from :var:`Init`
    :param init: The initial value for the scan. If not supplied, a default constructed object of type :type:`value_t\<Seq>` is used.

    :returns: A sequence adaptor which performs an inclusive scan of the elements of :var:`seq` using :var:`func`.

    :example:

    ..  literalinclude:: ../../example/docs/scan.cpp
        :language: cpp
        :dedent:
        :lines: 13-20

    :see also:
        * `std::partial_sum() <https://en.cppreference.com/w/cpp/algorithm/partial_sum>`_
        * `std::inclusive_scan() <https://en.cppreference.com/w/cpp/algorithm/inclusive_scan>`_
        * :func:`flux::scan_first`
        * :func:`flux::prescan`
        * :func:`flux::fold`

``scan_first``
^^^^^^^^^^^^^^

..  function::
    template <sequence Seq, typename Func> \
        requires foldable<Seq, Func, element_t<Seq>> \
    auto scan_first(Seq seq, Func func) -> sequence auto;

    Returns a stateful sequence adaptor which yields "partial folds" using the binary function :var:`func`.

    When iterated over, the returned sequence first initialises an internal variable ``state`` with the first element of the underlying sequence, and yields a read-only reference to this state. For each subsequent element ``elem``, it sets::

        state = func(std::move(state), std::forward(elem));

    and yields a read-only reference to the internal state. If :var:`seq` is empty, the internal state is never initialised and the resulting sequence is also empty. For a non-empty sequence, the final value yielded by :func:`scan_first` is the same as would be obtained from :expr:`fold_first(seq, func)`.

    Because this adaptor needs to maintain internal state, it is only ever single-pass. However it is a :concept:`bounded_sequence` when the underlying sequence is bounded and a :concept:`sized_sequence` when the underlying sequence is sized.

    Like :func:`scan`, this function performs an *inclusive scan*, that is, the Nth element of the adapted sequence includes the Nth element of the underlying sequence. The adapted sequence always yields the same number of elements as the underlying sequence. Unlike :func:`scan`, the first element of :func:`scan_first` is simply the first element of the underlying sequence, and the supplied :var:`func` is only applied to subsequent elements (this is equivalent to the differing behaviours of :func:`fold` and :func:`fold_first` respectively).

    :param seq: A sequence to adapt
    :param func: A binary callable of the form :expr:`R(R, element_t<Seq>)`, where :type:`R` is constructible from :expr:`element_t<Seq>`

    :returns: A sequence adaptor which performs an inclusive scan of the elements of :var:`seq` using :var:`func`.

    :example:

    ..  literalinclude:: ../../example/docs/scan_first.cpp
        :language: cpp
        :dedent:
        :lines: 13-21

    :see also:
        * `std::partial_sum() <https://en.cppreference.com/w/cpp/algorithm/partial_sum>`_
        * `std::inclusive_scan() <https://en.cppreference.com/w/cpp/algorithm/inclusive_scan>`_
        * :func:`flux::scan`
        * :func:`flux::fold_first`

``set_difference``
^^^^^^^^^^^^^^^^^^

..  function::
    template <sequence Seq1, sequence Seq2, typename Cmp = std::ranges::less> \
        requires strict_weak_order_for<Cmp, Seq1> && strict_weak_order_for<Cmp, Seq2> \
    auto set_difference(Seq1 seq1, Seq2 seq2, Cmp cmp = {}) -> sequence auto;

    Returns a sequence adaptor which yields the set difference of the two input sequences :var:`seq1` and :var:`seq2`, ordered by the given comparison function :var:`cmp`.

    This function assumes that both :var:`seq1` and :var:`seq2` are sorted with respect to the comparison function :var:`cmp`. If the input sequences are not sorted, the contents of the resulting sequence is unspecified.

    When the resulting sequence is iterated, it will output the elements from :var:`seq1` which are not found in the :var:`seq2` according to :var:`cmp`. If some element is found ``m`` times in :var:`seq1` and ``n`` times in :var:`seq2`, then the resulting sequence yields exactly ``std::max(m - n, 0)`` elements.

    :param seq1: The first sorted sequence.
    :param seq2: The second sorted sequence.
    :param cmp: A binary predicate that takes two elements as arguments and returns true if the first element is less than the second.

    :returns: A sequence adaptor that yields those elements of `seq1` which do not also appear in `seq2`.

    :example:

    ..  literalinclude:: ../../example/docs/set_difference.cpp
        :language: cpp
        :dedent:
        :lines: 14-19

    :see also:
        * `std::set_difference() <https://en.cppreference.com/w/cpp/algorithm/set_difference>`_
        * :func:`flux::set_symmetric_difference`
        * :func:`flux::set_intersection`
        * :func:`flux::set_union`

``set_intersection``
^^^^^^^^^^^^^^^^^^^^

..  function::
    template <sequence Seq1, sequence Seq2, typename Cmp = std::ranges::less> \
        requires strict_weak_order_for<Cmp, Seq1> && strict_weak_order_for<Cmp, Seq2> \
    auto set_intersection(Seq1 seq1, Seq2 seq2, Cmp cmp = {}) -> sequence auto;

    Returns a sequence adaptor which yields the set intersection of the two input sequences :var:`seq1` and :var:`seq2`, ordered by the given comparison function :var:`cmp`.

    This function assumes that both :var:`seq1` and :var:`seq2` are sorted with respect to the comparison function :var:`cmp`. If the input sequences are not sorted, the contents of the resulting sequence is unspecified.

    When the resulting sequence is iterated, it will output the elements from :var:`seq1` that are found in both sorted sequences according to :var:`cmp`. If some element is found ``m`` times in :var:`seq1` and ``n`` times in :var:`seq2`, then the resulting sequence yields exactly ``std::min(n, m)`` elements.

    :param seq1: The first sorted sequence.
    :param seq2: The second sorted sequence.
    :param cmp: A binary predicate that takes two elements as arguments and returns true if the first element is less than the second.

    :returns: A sequence adaptor that represents the set intersection of the two input sequences.

    :example:

    ..  literalinclude:: ../../example/docs/set_intersection.cpp
        :language: cpp
        :dedent:
        :lines: 14-19

    :see also:
        * `std::set_intersection() <https://en.cppreference.com/w/cpp/algorithm/set_intersection>`_
        * :func:`flux::set_difference`
        * :func:`flux::set_union`


``set_symmetric_difference``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

..  function::
    template <sequence Seq1, sequence Seq2, typename Cmp = std::ranges::less> \
        requires see_below \
    auto set_symmetric_difference(Seq1 seq1, Seq2 seq2, Cmp cmp = {}) -> sequence auto;

    Returns a sequence adaptor which yields the set symmetric difference of the two input sequences :var:`seq1` and :var:`seq2`, ordered by the given comparison function :var:`cmp`.

    This function assumes that both :var:`seq1` and :var:`seq2` are sorted with respect to the comparison function :var:`cmp`. If the input sequences are not sorted, the contents of the resulting sequence is unspecified.

    When the resulting sequence is iterated, it will output the elements that are found in either of the sequence, but not in both of them according to :var:`cmp`. If some element is found ``m`` times in :var:`seq1` and ``n`` times in :var:`seq2`, then the resulting sequence yields exactly ``std::abs(m - n)`` elements, preserving order:

    * if :expr:`m > n`, the final :expr:`m - n` of these elements from :var:`seq1`
    * if :expr:`m < n`, the final :expr:`n - m` of these elements from :var:`seq2`

    
    :requires:
        The expression in the ``requires`` clause is equivalent to::

            std::common_reference_with<element_t<Seq1>, element_t<Seq2>> &&
            std::common_reference_with<rvalue_element_t<Seq1>, rvalue_element_t<Seq2>> &&
            requires { typename std::common_type_t<value_t<Seq1>, value_t<Seq2>>; } &&
            strict_weak_order_for<Cmp, Seq1> &&
            strict_weak_order_for<Cmp, Seq2>

    :param seq1: The first sequence to merge.
    :param seq2: The second sequence to merge.
    :returns: A sequence adaptor that yields elements of `seq1` and `seq2` which do not appear in both sequences.

    :example:

    ..  literalinclude:: ../../example/docs/set_symmetric_difference.cpp
        :language: cpp
        :dedent:
        :lines: 14-19

    :see also:
        * `std::set_symmetric_difference() <https://en.cppreference.com/w/cpp/algorithm/set_symmetric_difference>`_
        * :func:`flux::set_difference`
        * :func:`flux::set_intersection`
        * :func:`flux::set_union`

``set_union``
^^^^^^^^^^^^^

..  function::
    template <sequence Seq1, sequence Seq2, typename Cmp = std::ranges::less> \
        requires see_below \
    auto set_union(Seq1 seq1, Seq2 seq2, Cmp cmp = {}) -> sequence auto;

    Returns a sequence adaptor which yields the set union of the two input sequences :var:`seq1` and :var:`seq2`, ordered by the given comparison function :var:`cmp`.

    This function assumes that both :var:`seq1` and :var:`seq2` are sorted with respect to the comparison function :var:`cmp`. If the input sequences are not sorted, the contents of the resulting sequence is unspecified.

    When the resulting sequence is iterated, it will output the elements from the two input sequences in order according to :var:`cmp`. If some element is found ``m`` times in :var:`seq1` and ``n`` times in :var:`seq2`, then the resulting sequence yields exactly ``std::max(n, m)`` elements.

    :requires:
        The expression in the ``requires`` clause is equivalent to::

            std::common_reference_with<element_t<Seq1>, element_t<Seq2>> &&
            std::common_reference_with<rvalue_element_t<Seq1>, rvalue_element_t<Seq2>> &&
            requires { typename std::common_type_t<value_t<Seq1>, value_t<Seq2>>; } &&
            strict_weak_order_for<Cmp, Seq1> &&
            strict_weak_order_for<Cmp, Seq2>
    
    :param seq1: The first sorted sequence to merge.
    :param seq2: The second sorted sequence to merge.
    :param cmp: A binary predicate that takes two elements as arguments and returns true if the first element is less than the second.

    :returns: A sequence adaptor that represents the set union of the two input sequences.

    :example:

    ..  literalinclude:: ../../example/docs/set_union.cpp
        :language: cpp
        :dedent:
        :lines: 14-19

    :see also:
        * `std::set_union() <https://en.cppreference.com/w/cpp/algorithm/set_union>`_
        * :func:`flux::set_intersection`
        * :func:`flux::set_difference`

``slide``
^^^^^^^^^

..  function::
    auto slide(multipass_sequence auto seq, std::integral auto win_sz) -> multipass_sequence auto;

``split``
^^^^^^^^^

..  function::
    template <multipass_sequence Seq, typename Delim> \
        requires std::equality_comparable_with<element_t<Seq>, Delim const&> \
    auto split(Seq seq, Delim delim) -> multipass_sequence auto;

..  function::
    template <multipass_sequence Seq, multipass_sequence Pattern> \
        requires std::equality_comparable_with<element_t<Seq>, element_t<Pattern>> \
    auto split(Seq seq, Pattern pattern) -> multipass_sequence auto;

..  function::
    template <multipass_sequence Seq, typename Pred> \
        requires std::predicate<Pred const&, element_t<seq>> \
    auto split(Seq seq, Pred pred) -> multipass_sequence auto;

    Splits a :concept:`multipass_sequence` into a sequence-of-subsequences using the given argument.

    The first overload takes a delimiter, which must be equality comparable with the source sequence's value type. The source sequence will be split on each occurrence of the delimiter, with the delimiter itself removed. Consecutive delimiters will result in empty subsequences in the output. If the source sequence begins with a delimiter then the first subsequence will be empty, and likewise if it ends with a delimiter then the final subsequence will be empty.

    The second overload takes another sequence, the :var:`pattern`, whose elements must be equality comparable with the elements of the source sequence. The source is split whenever the pattern occurs as a subsequence. Consecutive (non-overlapping) occurrences of the pattern will result in empty sequences in the output. If :expr:`ends_with(seq, pattern)` is :expr:`true`, the final subsequence will be empty.

    The third overload takes a unary predicate which will be called with successive elements of the source sequence and returns :expr:`true` when a split should occur. The "``true``" element will be removed from the output. If the predicate returns ``true`` for two consecutive of the source, then the output will contain an empty subsequence. If the predicate returns ``true``` for the final element of the source, then the final subsequence will be empty.

    The returned sequence is always a :concept:`multipass_sequence`. It is additionally a :concept:`bounded_sequence` when :var:`Seq` is bounded.

    :param seq: A multipass sequence to split.
    :param delim: For the first overload, a delimiter to split on. Must be equality comparable with the element type of :var:`seq`
    :param pattern: For the second overload, a multipass sequence to split on. Its element type must be equality comparable with the element type of :var:`seq`.
    :param pred: For the third overload, a unary predicate accepting elements of :var:`seq`, returning ``true`` when a split should occur.

    :returns: A multipass sequence whose elements are subsequences of :var:`seq`.

    :example:

    ..  literalinclude:: ../../example/docs/split.cpp
        :language: cpp
        :dedent:
        :lines: 18-79

    :see also:
        * `std::views::split() <https://en.cppreference.com/w/cpp/ranges/split_view>`_
        * :func:`flux::chunk_by`

``stride``
^^^^^^^^^^

..  function::
    auto stride(sequence auto seq, std::integral auto stride_len) -> sequence auto;

``take``
^^^^^^^^

..  function::
    auto take(sequence auto seq, std::integral auto count) -> sequence auto;

``take_while``
^^^^^^^^^^^^^^

..  function::
    template <sequence Seq, typename Pred> \
        requires std::predicate<Pred&, element_t<Seq>> \
    auto take_while(Seq seq, Pred pred) -> sequence auto;

``unchecked``
^^^^^^^^^^^^^

..  function::
    auto unchecked(sequence auto seq) -> sequence auto;

``zip``
^^^^^^^

..  function::
    auto zip(sequence auto... seqs) -> sequence auto;