//
//  UIScrollView+InfiniteScroll.h
//
//  UIScrollView infinite scroll category
//
//
//  The MIT License (MIT)
//
//  Copyright (c) 2013-2015 Andrei Mihailov
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//
//  Created by Andrej Mihajlov on 9/4/13.
//  Copyright (c) 2013-2015 Andrej Mihajlov. All rights reserved.
//

@interface UIScrollView (InfiniteScroll)

/**
 *  Flag that indicates whether infinite scroll is animating
 */
@property (nonatomic, readonly, getter=isAnimatingInfiniteScroll) BOOL animatingInfiniteScroll;

/**
 *  Infinite scroll activity indicator style (default: UIActivityIndicatorViewStyleGray on iOS, UIActivityIndicatorViewStyleWhite on tvOS)
 */
@property (nonatomic) UIActivityIndicatorViewStyle infiniteScrollIndicatorStyle;

/**
 *  Infinite indicator view
 *
 *  You can set your own custom view instead of default activity indicator, 
 *  make sure it implements methods below:
 *
 *  * `- (void)startAnimating`
 *  * `- (void)stopAnimating`
 *
 *  Infinite scroll will call implemented methods during user interaction.
 */
@property (nonatomic, nullable) UIView *infiniteScrollIndicatorView;

/**
 *  Vertical margin around indicator view (Default: 11)
 */
@property (nonatomic) CGFloat infiniteScrollIndicatorMargin;

/**
 *  Setup infinite scroll handler
 *
 *  @param handler a handler block
 */
- (void)addInfiniteScrollWithHandler:(nonnull void(^)(__kindof UIScrollView * __nonnull scrollView)) handler;

/**
 *  Unregister infinite scroll
 */
- (void)removeInfiniteScroll;

/**
 *  Finish infinite scroll animations
 *
 *  You must call this method from your infinite scroll handler to finish all
 *  animations properly and reset infinite scroll state
 *
 *  @param handler a completion block handler called when animation finished
 */
- (void)finishInfiniteScrollWithCompletion:(nullable void(^)(__kindof UIScrollView * __nonnull scrollView))handler;

/**
 *  Finish infinite scroll animations
 *
 *  You must call this method from your infinite scroll handler to finish all
 *  animations properly and reset infinite scroll state
 */
- (void)finishInfiniteScroll;

@end
