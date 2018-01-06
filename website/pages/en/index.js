/**
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const React = require('react');

const CompLibrary = require('../../core/CompLibrary.js');
const MarkdownBlock = CompLibrary.MarkdownBlock; /* Used to read markdown */
const Container = CompLibrary.Container;
const GridBlock = CompLibrary.GridBlock;

const siteConfig = require(process.cwd() + '/siteConfig.js');

class Button extends React.Component {
  render() {
    return (
      <div className="pluginWrapper buttonWrapper" id={this.props.key}>
        <a className="button" href={this.props.href} target={this.props.target}>
          {this.props.children}
        </a>
      </div>
    );
  }
}

Button.defaultProps = {
  target: '_self',
};

class HomeSplash extends React.Component {
  render() {
    return (
      <div className="homeContainer">
        <div className="homeSplashFade">
          <div className="wrapper homeWrapper">
            <div className="projectLogo">
              <img src={siteConfig.baseUrl + 'img/AppImageUpdaterBridge.png'} />
            </div>
            <div className="inner">
              <h2 className="projectTitle">
                {siteConfig.title}
                <small>{siteConfig.tagline}</small>
              </h2>
              <div className="section promoSection">
                <div className="promoRow">
                  <div className="pluginRowBlock">
                    <Button
                      href={
                        siteConfig.baseUrl +
                        'docs/' +
                        'Installation.html'
                      }>
                      Get Started
                    </Button>
                    <Button
                      href={
                        siteConfig.baseUrl +
                        'docs/' +
                        'AppImageUpdaterBridgeErrorCodes.html'
                      }>
                      API Reference
                    </Button>
	            <Button href={'https://github.com/antony-jr/AppImageUpdaterBridge'}>
	    	      View on Github
	    	    </Button>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    );
  }
}

class Index extends React.Component {
  render() {
    let language = this.props.language || 'en';
    const showcase = siteConfig.users
      .filter(user => {
        return user.pinned;
      })
      .map(user => {
        return (
          <a href={user.infoLink}>
            <img src={user.image} title={user.caption} />
          </a>
        );
      });

    return (
      <div>
        <HomeSplash language={language} />
        <div className="mainContainer">
          <Container padding={['bottom', 'top']}>
            <GridBlock
              align="center"
              contents={[
                {
                  content: 'AppImage Updater Bridge fully supports Qt Projects by flowing with its event loop and'+
			   ' thus you will have no problem integrating AppImage Updater Bridge with your Qt Project.',
                  image: siteConfig.baseUrl + 'img/qt.png',
                  imageAlign: 'top',
                  title: 'Supports Qt\'s Event Loop.',
                },
                {
                  content: 'Read Update Information Directly from your AppImage with a very clean API , without any'+
                            ' OS Specifics. Thus giving the best results on extracting Update Information from any'+
                            ' AppImage.',
                  image: siteConfig.baseUrl + 'img/bin.png',
                  imageAlign: 'top',
                  title: 'Get Update Information Directly from your AppImage!',
                },
                {
                  content: 'This library was not only designed to be powerfull but also light weight than the official'+
                           ' AppImage Updater library , As this library only depends on Qt5 , as long as you use Qt5,' +
                           ' This library will never seem like a weight to you.',
                   image: siteConfig.baseUrl + 'img/light.png',
                   imageAlign: 'top',
                   title: 'Very Light Wieght!',
                },
                {
                  content: 'This project is rated by <b>Codacy</b> with a <b>A Project Certification</b>' +
                           ' and thus integrating this library will not affect your source , So this project is also'+
                           ' best suited for those who care about code taste.',
                  image: siteConfig.baseUrl + 'img/clean_code.png',
                  imageAlign: 'top',
                  title: 'Clean C++ API.',
                },

              ]}
              layout="fourColumn"
            />
          </Container>

          <Container padding={['bottom', 'top']} background="dark">
            <GridBlock
              contents={[
                {
                  content:
                    ' AppImage\'s are cool but when it comes to updating package that uses AppImage , it can be pretty hard to workout.' +
                    ' And Therefore I started creating a lot of libraries that can be used to AutoUpdate any software including AppImages , ' +
                    ' But AppImages used a seperate efficient way to update packages , Thus I started working on creating a nice bridge to'+
                    ' this mechanism and So AppImage Updater Bridge was born.',
                  title: 'Why Create AppImageUpdaterBridge ?',
                },
              ]}
            />
          </Container>

          <div className="productShowcaseSection paddingBottom">
            <h2>{"Who's Using This?"}</h2>
            <p>This project is used by all these people</p>
            <div className="logos">{showcase}</div>
            <div className="more-users">
              <a
                className="button"
                href={
                  siteConfig.baseUrl + this.props.language + '/' + 'users.html'
                }>
                More {siteConfig.title} Users
              </a>
            </div>
          </div>
        </div>
      </div>
    );
  }
}

module.exports = Index;
