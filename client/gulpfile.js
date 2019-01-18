/**
 * @file
 * Gulp build runner
 *
 * IDEA gulp.src('./dist/coregrounds.js').pipe(gulp.dest('./apps/windows-debug/package.nw/'));
 */
const gulp = require('gulp');
const sass = require('gulp-sass');
const glob = require('gulp-sass-glob');
const sourcemaps = require('gulp-sourcemaps');
const rollup = require('rollup');
const uglify = require('rollup-plugin-uglify');
const json = require('rollup-plugin-json');
const commonjs = require('rollup-plugin-commonjs');
const resolve = require('rollup-plugin-node-resolve');
const string = require('rollup-plugin-string');
const beep = require('beeper');

gulp.task('rollup', () => {
  // Dev version
  rollup
    .rollup({
      input: './src/js/App.js',
      plugins: [
        resolve(),
        commonjs(),
        string({
          include: '**/*.svg',
        }),
        json(),
      ],
    })
    .then(bundle => {
      bundle.write({
        format: 'umd',
        file: './apps/windows-debug/package.nw/coregrounds.js',
        sourcemap: true,
      });
    })
    .catch(err => {
      beep(3);
      console.error(err);
    });
  // Dist version
  rollup
    .rollup({
      input: './src/js/App.js',
      plugins: [
        resolve(),
        commonjs(),
        string({
          include: '**/*.svg',
        }),
        json(),
        uglify(),
      ],
    })
    .then((bundle) => {
      bundle.write({
        format: 'umd',
        file: './dist/coregrounds.js',
        sourcemap: false,
      });
    });
});

gulp.task('sass', () => {
  gulp
    .src('./src/sass/styles.scss')
    .pipe(sourcemaps.init())
    .pipe(glob())
    .pipe(
      sass({
        includePaths: ['./src/sass'],
        outputStyle: 'compressed',
      }).on('error', sass.logError)
    )
    .pipe(sourcemaps.write('./'))
    .pipe(gulp.dest('./dist'))
    .pipe(gulp.dest('./apps/windows-debug/package.nw'));
});

gulp.task('default', () => {
  gulp.watch('src/sass/**/*.scss', ['sass']);
  gulp.watch('src/js/**/*.*', ['rollup']);
  gulp.watch('apps/windows-debug/package.nw/coregrounds.js', ['beep']);
});

gulp.task('beep', () => {
  beep();
});

gulp.task('dist', () => {
  gulp.src(['./dist/**/*']).pipe(gulp.dest('./apps/windows/package.nw'));
  gulp.src(['./dist/**/*']).pipe(gulp.dest('./apps/windows-32bit/package.nw'));
  gulp.src(['./dist/**/*']).pipe(gulp.dest('./apps/windows-debug/package.nw'));
  gulp.src(['./dist/**/*']).pipe(gulp.dest('./apps/linux/package.nw'));
  gulp
    .src(['./dist/**/*'])
    .pipe(gulp.dest('./apps/darwin/coregrounds.app/Contents/Resources/app.nw'));
});
